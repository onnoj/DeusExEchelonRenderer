package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"io/fs"
	"log"
	"math/rand/v2"
	"os"
	"path"
	"path/filepath"
	"slices"
	"strings"
	"sync"
	"time"

	"github.com/onnoj/DeusExEchelonRenderer/OmniverseHelper/data"
	. "github.com/onnoj/DeusExEchelonRenderer/OmniverseHelper/internal"
	"github.com/onnoj/DeusExEchelonRenderer/OmniverseHelper/utils"
)

// constants:
const TEXTURES_PER_BATCH = 16

// Global variables
var (
	PackageNames       []string                            = make([]string, 0)
	TextureMapping     map[uint64]*data.TextureData        = make(map[uint64]*data.TextureData)
	TextureFileMapping map[string]*data.TextureData        = make(map[string]*data.TextureData)
	PackageMapping     map[string]data.TextureDataPtrSlice = make(map[string]data.TextureDataPtrSlice)
	LayerMapping       map[string]Layer                    = make(map[string]Layer)

	ProjectLayer Layer
	RootLayer    Layer
	RootDir      string
)

func normalizeLayerPath(layerPath string) string {
	p, _ := filepath.Abs(layerPath)
	return p
}

func findLayer(layerName string, isRelative bool) *Layer {
	var id string
	if isRelative {
		id = normalizeLayerPath(path.Join(RootDir, layerName))
	} else {
		id = normalizeLayerPath(layerName)
	}

	layer, found := LayerMapping[id]
	if found {
		return &layer
	}
	return nil
}

func loadLayers(c *RestClient) {
	layers, err := c.GetLayers(nil, -1)
	if err != nil {
		log.Fatalf("Unable to fetch layers: %s", err.Error())
	}
	if len(layers.Layers) > 0 {
		ProjectLayer = layers.Layers[0]
		if ProjectLayer.LayerType != string(LayerType_Workfile) {
			log.Fatalf("Layer '%s' was not the root workfile.", ProjectLayer.LayerFilePath)
		}
		RootDir = normalizeLayerPath(filepath.Dir(ProjectLayer.LayerFilePath))
	} else {
		log.Fatal("Could not set project layer.")
	}

	foundRootLayer := false
	for _, l := range ProjectLayer.Children {
		if l.LayerType == string(LayerType_Replacement) {
			RootLayer = l
			foundRootLayer = true
			break
		}
	}
	if !foundRootLayer {
		log.Fatal("Could not set root work layer.")
	}

	LayerMapping = make(map[string]Layer)
	var processLayer func(layer Layer)
	processLayer = func(layer Layer) {
		LayerMapping[normalizeLayerPath(layer.LayerFilePath)] = layer
		for _, v := range layer.Children {
			processLayer(v)
		}
	}
	processLayer(ProjectLayer)
}

func createLayer(c *RestClient, params PostCreateLayerRequestParams) (Layer, error) {
	res, err := c.PostCreateLayer(params)
	if err != nil || res != `"OK"` {
		return Layer{}, fmt.Errorf("create layer failed, %s", err.Error())
	}

	loadLayers(c)
	l := findLayer(params.LayerPath, false)
	if l != nil {
		return *l, nil
	}
	return Layer{}, errors.New("create layer succeeded but was unable to get the result")
}

// ExecuteMassIngest adds all child commands to the root command and sets flags appropriately.
func ExecuteMassIngest() {
	//Set up rest client
	httpClient := NewClient("http://127.0.0.1:8011", time.Minute*15)
	loadLayers(httpClient)
	httpClient.DebugMode = false

	//Prepare the texturemapping
	jsonFile, err := os.Open(RootCmdOptions.TextureDumpJSON)
	if err != nil {
		log.Fatalf("Could not open '%s', exiting.", RootCmdOptions.TextureDumpJSON)
	}
	var dump JSONTextureObj
	jsonData, _ := io.ReadAll(jsonFile)
	if err := json.Unmarshal(jsonData, &dump); err != nil {
		log.Fatalf("Could not deserialize '%s'", RootCmdOptions.TextureDumpJSON)
	}
	for _, texInfo := range dump.Textures {
		wrappedTexInfo := data.FromJSONTextureInfo(texInfo)
		wrappedTexInfo.RemixHashText = strings.ToUpper(wrappedTexInfo.RemixHashText)
		TextureMapping[texInfo.RemixHash] = &wrappedTexInfo
		PackageMapping[texInfo.PackageName] = append(PackageMapping[texInfo.PackageName], &wrappedTexInfo)
		PackageNames = utils.AppendUnique(PackageNames, texInfo.PackageName)
	}

	//(Recursively) walk the given folder, looking for files to process.
	type ProcessFolderFunc = func(path string, d fs.DirEntry, err error) error
	var processFolder ProcessFolderFunc
	processFolder = func(path string, d fs.DirEntry, err error) error {
		path, _ = filepath.Abs(path)
		if d.IsDir() {
			rel, _ := filepath.Rel(RootCmdOptions.InputFolder, d.Name())
			if IngestCmdOptions.InputFolderRecursive && rel != "" {
				if err := filepath.WalkDir(RootCmdOptions.InputFolder, processFolder); err != nil {
					log.Fatal(err.Error())
				}
			} else {
				return nil
			}
		}

		var textureType data.TextureType
		var hashIdentifier uint64
		var remainder string
		_, err = fmt.Sscanf(d.Name(), "%X_%s", &hashIdentifier, &remainder)
		if mappedTextureData, ok := TextureMapping[hashIdentifier]; ok {
			if strings.HasPrefix(remainder, "diffuse") {
				textureType = data.TextureType_Albedo
			} else if strings.HasPrefix(remainder, "roughness") {
				textureType = data.TextureType_Roughness
			} else if strings.HasPrefix(remainder, "normal_dx") {
				textureType = data.TextureType_NormalDX
			} else if strings.HasPrefix(remainder, "normal") {
				textureType = data.TextureType_NormalDX
			} else if strings.HasPrefix(remainder, "height") {
				textureType = data.TextureType_Height
			} else {
				log.Fatal("option not implemented, please fix me")
			}
			TextureFileMapping[path] = mappedTextureData
			mappedTextureData.ImportTexturePath[uint(textureType)] = path
		}
		return nil
	}
	if err := filepath.WalkDir(RootCmdOptions.InputFolder, processFolder); err != nil {
		log.Fatal(err.Error())
	}

	defaultAssetDirectory, err := httpClient.GetAssetsDefaultDirectory()
	if err != nil {
		log.Fatal(err.Error())
	}
	defaultAssetDirectory, _ = filepath.Abs(defaultAssetDirectory)

	//Start processing each package.
	for packageIdx, packageName := range PackageNames {
		if !utils.MatchAnyOfRegex(IngestCmdOptions.PackageFiltersRegexes, packageName) {
			continue
		}
		fmt.Printf("Processing package %s\t%d/%d", packageName, packageIdx, len(PackageNames))

		totalProcessedTextureCount := 0
		layerName := fmt.Sprintf("Overrides_%s.usda", packageName)
		activeLayer := prepareLayer(layerName, httpClient)
		workloads, totaltextureCount := generateIngestionWorkload(packageName, defaultAssetDirectory, httpClient)
		for _, workload := range workloads {
			ingestResults, processedTextureCount := executeTexturesIngestion(*workload, activeLayer, httpClient)

			if len(ingestResults) > 0 {
				progressf := float64(totalProcessedTextureCount) / float64(totaltextureCount)
				log.Printf("Processed %d textures of %d remaining; %d%%.\n", len(workload.InputFiles), (totaltextureCount - totalProcessedTextureCount), uint(progressf*100.0))

				totalProcessedTextureCount += processedTextureCount
				inputOutputMapping := processIngestResults(*workload, ingestResults)
				processTextureOverrides(inputOutputMapping, httpClient)

				// Save the layer after processing
				if _, err := httpClient.SaveLayer(activeLayer.LayerFilePath); err != nil {
					log.Fatalf("Was not able to save layer: %s", err.Error())
				}
			}
		}
	}
}

func prepareLayer(layerName string, httpClient *RestClient) *Layer {
	activeLayer := findLayer(layerName, true)
	if activeLayer == nil {
		layerPath := path.Join(RootDir, layerName)
		if _, err := os.Stat(layerPath); err == nil {
			os.Remove(layerPath)
		}
		l, err := createLayer(httpClient, PostCreateLayerRequestParams{
			LayerPath:       layerPath,
			LayerType:       LayerType_AutoUpscale,
			SetEditTarget:   utils.NewPtrValue(true),
			ParentLayerID:   utils.NewPtrValue(RootLayer.LayerFilePath),
			CreateOrInsert:  utils.NewPtrValue(true),
			ReplaceExisting: utils.NewPtrValue(false),
		})
		if err != nil {
			log.Fatal(err.Error())
		}
		activeLayer = &l
	}

	if _, err := httpClient.SetEditLayer(activeLayer.LayerFilePath); err != nil {
		log.Fatalf("Failed to switch to layer %s", activeLayer.LayerFilePath)
	}
	return activeLayer
}

func generateIngestionWorkload(packageName string, defaultAssetDirectory string, httpClient *RestClient) ([]*IngestTexturesRequestParams, int) {
	totalTextureCount := 0
	params := make([]*IngestTexturesRequestParams, 0)
	var currentParams *IngestTexturesRequestParams
	allocateNewParams := func() {
		currentParams = new(IngestTexturesRequestParams)
		currentParams.OutputDirectory = path.Join(defaultAssetDirectory, "overrides", packageName)
		params = append(params, currentParams)
	}
	allocateNewParams()

	// Iterate over textures in the package
	remixTextures, err := httpClient.GetTextures(GetTexturesRequestParams{})
	if err != nil {
		log.Fatalln("Error: could not fetch textures.")
		return nil, 0
	}

	for _, v := range PackageMapping[packageName] {
		// Process texture paths
		for i := data.TextureType(0); i < data.TextureType_COUNT; i++ {
			path := v.ImportTexturePath[i]
			if len(path) > 0 {
				// Check if texture has already been converted
				if !IngestCmdOptions.OverwriteOverrides {
					expectedRemixFilePath := fmt.Sprintf("/RootNode/Looks/mat_%s/Shader.inputs:%s", v.RemixHashText, data.TextureTypeToShaderInput(i))
					remixFilePathTuple := utils.SelectFromSlice(remixTextures.Textures, func(p [2]string) bool {
						return (p[0] == expectedRemixFilePath)
					})

					if remixFilePathTuple != nil {
						remixFilePath, _ := filepath.Abs((*remixFilePathTuple)[1])
						if strings.HasPrefix(remixFilePath, defaultAssetDirectory) &&
							strings.Contains(remixFilePath, data.TextureTypeToFilenameGlob(i)) {
							continue
						}
					}
				}

				// Add texture to input for conversion
				currentParams.InputHashes = append(currentParams.InputHashes, v.RemixHash)
				currentParams.InputFiles = append(currentParams.InputFiles, v.ImportTexturePath[i])
				currentParams.InputTextureTypeStrings = append(currentParams.InputTextureTypeStrings, data.TextureTypeToString(data.TextureType(i)))
				currentParams.InputTextureTypePathGlobs = append(currentParams.InputTextureTypePathGlobs, data.TextureTypeToFilenameGlob(data.TextureType(i)))
				totalTextureCount++

				// If batch size is reached, allocate new parameters
				if len(currentParams.InputFiles) >= TEXTURES_PER_BATCH {
					allocateNewParams()
				}
			}
		}
	}

	return params, totalTextureCount
}

// executeTexturesIngestion triggers the actual ingestion process
func executeTexturesIngestion(workload IngestTexturesRequestParams, activeLayer *Layer, httpClient *RestClient) (map[string]IngestTexturesResponse, int) {
	log.Println("Starting texture ingestion...")

	// Mutex to lock results map during concurrent access
	ingestResultsLock := sync.Mutex{}
	ingestResults := make(map[string]IngestTexturesResponse)
	batcher := utils.NewJobScheduler(1)
	defer batcher.Close()

	// Process each parameter set
	if len(workload.InputFiles) == 0 {
		return nil, 0
	}

	// Schedule texture ingestion jobs
	processedTextureCount := 0
	batcher.Schedule(
		func() {
			var lastErr error
			for i := uint(0); i < 5; i++ { // Retry up to 5 times
				if obj, err := httpClient.IngestTextures(workload); err == nil {
					// Successful ingestion
					ingestResultsLock.Lock()
					ingestResults[strings.Join(workload.InputFiles, ";")] = obj
					ingestResultsLock.Unlock()

					processedTextureCount += len(workload.InputFiles)
					break
				} else {
					// Handle retry
					lastErr = err
					log.Printf("Failed to process textures: %+v\n", workload.InputFiles)
					log.Printf("error was: %s\n", err.Error())
					log.Printf("ingestTexture failed %d times, retrying...\n", i+1)
					time.Sleep(time.Second * time.Duration(5+rand.IntN(15)))
				}
			}
			// Log final error if all retries fail
			if lastErr != nil {
				log.Printf("Failed to process textures: %+v", workload.InputFiles)
				log.Printf("IngestTextures REST call failed with: %s", lastErr.Error())
			}
		})

	// Wait for all tasks to be done.
	batcher.Wait()

	// Save the layer after processing
	if _, err := httpClient.SaveLayer(activeLayer.LayerFilePath); err != nil {
		log.Fatalf("Was not able to save layer: %s", err.Error())
	}

	return ingestResults, processedTextureCount
}

// ProcessIngestResults processes ingestion results and maps input to output file paths.
func processIngestResults(originalRequestParam IngestTexturesRequestParams, ingestResults map[string]IngestTexturesResponse) map[string]string {
	// Initialize the mapping of input to output paths
	inputOutputMapping := map[string]string{}

	for identifier, r := range ingestResults {
		for _, completedTasks := range r.CompletedSchemas {
			plugin := utils.SelectFromSlice(completedTasks.CheckPlugins, func(eval IngestTexturesResponseCheckPlugin) bool {
				return (eval.Name == "ConvertToDDS")
			})
			if plugin == nil {
				log.Printf("WARNING: Unable to locate the ConvertToDDS plugin of ingestion task '%s'", identifier)
				continue
			}

			inputFlow := utils.SelectFromSlice(completedTasks.ContextPlugin.Data.DataFlows, func(eval IngestTexturesResponseDataFlow) bool {
				return (eval.Name == "InOutData" && eval.Channel == "Default")
			})

			outputFlow := utils.SelectFromSlice(plugin.Data.DataFlows, func(eval IngestTexturesResponseDataFlow) bool {
				// Return the cleanup_files channel as it conveniently has both input and output files.
				// Otherwise, use ingestion_output.
				return (eval.Name == "InOutData" && eval.Channel == "cleanup_files")
			})
			if outputFlow == nil {
				log.Printf("WARNING: Unable to locate the InOutData.cleanup_files field of ingestion task '%s'", identifier)
				continue
			}

			var outputData []string
			if slice, convertedOk := outputFlow.OutputData.([]interface{}); convertedOk {
				for _, s := range slice {
					if s, ok := s.(string); ok {
						outputData = append(outputData, s)
					} else {
						log.Panicln("ERROR: Unexpected non-string data in output flow.")
					}
				}
			} else {
				log.Printf("WARNING: Could not convert the dataflow's output_data field to a string array.")
				continue
			}

			// Ensure there's at least one output and input path
			if len(outputData) < 1 {
				log.Panicf("ERROR: Should have received one or more output path, but received %d", len(outputData))
			}
			if len(outputFlow.InputData) < 1 {
				log.Panicf("ERROR: Should have received one or more input path, but received %d", len(outputFlow.InputData))
			}

			// Validate that the number of inputs and outputs are consistent
			if len(outputFlow.InputData) != len(outputData) || len(inputFlow.InputData) != len(outputFlow.InputData) {
				log.Panicf("ERROR: Mismatch between number of inputs and outputs received")
			}

			// Map input paths to output paths
			// The order in the returned arrays is sadly not guaranteed...
			// So for each input, we'll try to find a matching output based on materialid+texture type.
			for i := 0; i < len(inputFlow.InputData); i++ {
				var inputHash uint64
				var glob string
				inputPath, _ := filepath.Abs(inputFlow.InputData[i])

				//Acquire the correct index into the originalRequestParam slices by matching the known input path
				idx := slices.Index(originalRequestParam.InputFiles, inputPath)
				if idx >= 0 && idx < len(originalRequestParam.InputTextureTypePathGlobs) {
					glob = originalRequestParam.InputTextureTypePathGlobs[idx]
					inputHash = originalRequestParam.InputHashes[idx]
				} else {
					log.Fatal("Slice access out of bounds.")
				}

				//Look up the matching textureInfo so we can get the (correctly formatted) hash
				textureInfo := TextureMapping[inputHash]
				outputPath := *utils.SelectFromSlice(outputData, func(eval string) bool {
					return strings.Contains(eval, textureInfo.RemixHashText) && strings.Contains(eval, glob)
				})

				//Normalize the output path and store it in the lookup mapping for later use.
				outputPath, _ = filepath.Abs(outputPath)
				inputOutputMapping[inputPath] = outputPath
			}
		}
	}

	return inputOutputMapping
}

// ProcessTextureOverrides processes texture mappings and sets override textures
func processTextureOverrides(inputOutputMapping map[string]string, httpClient *RestClient) {
	fmt.Println("Done with package, setting overrides...")

	for key, val := range inputOutputMapping {
		if info, found := TextureFileMapping[key]; found {
			for i := data.TextureType(0); i < data.TextureType_COUNT; i++ {
				if info.ImportTexturePath[i] == key {
					diffusePath := fmt.Sprintf("/RootNode/Looks/mat_%s/Shader.inputs:%s", info.RemixHashText, data.TextureTypeToShaderInput(1))
					httpClient.ValidateTextureMaterialInputs(diffusePath)

					path := fmt.Sprintf("/RootNode/Looks/mat_%s/Shader.inputs:%s", info.RemixHashText, data.TextureTypeToShaderInput(i))
					if _, err := httpClient.ValidateTextureMaterialInputs(path); err != nil {
						log.Printf("[Warning] Failed to validate material inputs of '%s', error: %s", path, err.Error())
					}

					expectedFilenameTextureType := data.TextureTypeToFilenameGlob(i)
					if !strings.Contains(val, expectedFilenameTextureType) {
						log.Fatalf("replacement texture %s did not match expected texture type of *%s*", val, expectedFilenameTextureType)
					}

					_, err := httpClient.SetOverrideTextures([]string{path}, []string{val}, true)
					if err != nil {
						log.Printf("Failed to set override textures, %s", err.Error())
					}

					fmt.Printf("%s\t%s\t%s\n", info.RemixHashText, data.TextureTypeToString(i), val)
				}
			}
		} else {
			log.Panicf("Could not find info for file %s", key)
		}
	}

	fmt.Println("Done with package, setting overrides: done")
}
