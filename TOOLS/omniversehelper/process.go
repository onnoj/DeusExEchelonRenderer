package main

import (
	"encoding/json"
	"fmt"
	"io"
	"io/fs"
	"log"
	"math/rand/v2"
	"os"
	"path"
	"path/filepath"
	"strings"
	"time"

	"github.com/onnoj/DeusExEchelonRenderer/OmniverseHelper/data"
	. "github.com/onnoj/DeusExEchelonRenderer/OmniverseHelper/internal"
	"github.com/onnoj/DeusExEchelonRenderer/OmniverseHelper/utils"
)

// constants:
const TEXTURES_PER_BATCH = 1

// Global variables
var (
	PackageNames   []string                            = make([]string, 0)
	TextureMapping map[uint64]*data.TextureData        = make(map[uint64]*data.TextureData)
	PackageMapping map[string]data.TextureDataPtrSlice = make(map[string]data.TextureDataPtrSlice)
	LayerMapping   map[string]Layer                    = make(map[string]Layer)

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
	return Layer{}, fmt.Errorf("create layer succeeded but was unable to get the result, %s", err.Error())
}

// Execute adds all child commands to the root command and sets flags appropriately.
func Execute() {
	//Set up rest client
	httpClient := NewClient("http://127.0.0.1:8011", time.Minute*15)
	loadLayers(httpClient)

	/*
		ass, err := httpClient.GetAssets(GetAssetsQueryParams{})
		if err != nil {
			log.Fatal(err.Error())
		}
		log.Println(ass)
		os.WriteFile("d:\\temp\\mongoose.txt", []byte(strings.Join(ass, "\r\n")), 0777)
	*/

	resp, err := httpClient.GetTextures(GetTexturesRequestParams{})
	if err != nil {
		log.Fatal(err.Error())
	}
	log.Println(resp.Textures)

	f, _ := os.OpenFile("d:\\temp\\mongoose.txt", os.O_CREATE|os.O_WRONLY, 0777)
	for _, t := range resp.Textures {
		f.WriteString(fmt.Sprintf("%s\t\t%s\r\n", t[0], t[1]))
	}
	f.Close()

	//Prepare the texturemapping
	jsonFile, err := os.Open(Options.TextureDumpJSON)
	if err != nil {
		log.Fatalf("Could not open '%s', exiting.", Options.TextureDumpJSON)
	}
	var dump JSONTextureObj
	jsonData, _ := io.ReadAll(jsonFile)
	if err := json.Unmarshal(jsonData, &dump); err != nil {
		log.Fatalf("Could not deserialize '%s'", Options.TextureDumpJSON)
	}
	for _, texInfo := range dump.Textures {
		wrappedTexInfo := data.FromJSONTextureInfo(texInfo)
		TextureMapping[texInfo.RemixHash] = &wrappedTexInfo
		PackageMapping[texInfo.PackageName] = append(PackageMapping[texInfo.PackageName], &wrappedTexInfo)
		PackageNames = utils.AppendUnique(PackageNames, texInfo.PackageName)
	}

	//(Recursively) walk the given folder, looking for files to process.
	type ProcessFolderFunc = func(path string, d fs.DirEntry, err error) error
	var processFolder ProcessFolderFunc
	processFolder = func(path string, d fs.DirEntry, err error) error {
		if d.IsDir() {
			rel, _ := filepath.Rel(Options.InputFolder, d.Name())
			if Options.InputFolderRecursive && rel != "" {
				if err := filepath.WalkDir(Options.InputFolder, processFolder); err != nil {
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
			} else {
				log.Fatal("option not implemented, please fix me")
			}

			mappedTextureData.ImportTexturePath[uint(textureType)] = path
		}
		return nil
	}
	if err := filepath.WalkDir(Options.InputFolder, processFolder); err != nil {
		log.Fatal(err.Error())
	}

	defaultAssetDirectory, err := httpClient.GetAssetsDefaultDirectory()
	if err != nil {
		log.Fatal(err.Error())
	}

	//Start processing each package.
	for _, packageName := range PackageNames {
		if !utils.MatchAnyOfRegex(Options.PackageFiltersRegexes, packageName) {
			continue
		}

		layerName := fmt.Sprintf("Overrides_%s.usda", packageName)
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

		params := make([]*IngestTexturesRequestParams, 0)
		var currentParams *IngestTexturesRequestParams
		allocateNewParams := func() {
			currentParams = new(IngestTexturesRequestParams)
			currentParams.OutputDirectory = path.Join(defaultAssetDirectory, "overrides", packageName)
			params = append(params, currentParams)
		}
		allocateNewParams()

		totalTextureCount := 0
		for _, v := range PackageMapping[packageName] {
			for i := 0; i < int(data.TextureType_COUNT); i++ {
				path := v.ImportTexturePath[i]
				if len(path) > 0 {
					currentParams.InputFiles = append(currentParams.InputFiles, struct {
						InputFilePath string
						TextureType   string
					}{
						InputFilePath: v.ImportTexturePath[i],
						TextureType:   data.TextureTypeToString(data.TextureType(i)),
					})

					if len(currentParams.InputFiles) >= TEXTURES_PER_BATCH {
						allocateNewParams()
					}
				}
			}

			totalTextureCount += len(currentParams.InputFiles)
		}

		batcher := utils.NewJobScheduler(8)
		processedTextureCount := 0
		for _, p := range params {
			//progressf := (float64(processedTextureCount) / float64(totalTextureCount))
			//log.Printf("Processing package '%s', %d textures, %d%%.", packageName, len(p.InputFiles), uint(progressf*100.0))
			if len(p.InputFiles) == 0 {
				continue
			}

			batcher.Schedule(
				func() {
					var lastErr error
					for i := uint(0); i < 5; i++ {
						if _, err := httpClient.IngestTextures(*p); err == nil {
							//success!
							lastErr = err
							break
						} else {
							//try one more time...
							lastErr = err
							log.Printf("Failed to process textures: %+v", p.InputFiles)
							log.Printf("error was: %s", err.Error())
							log.Printf("ingestTexture failed %d times, retrying...", i+1)
							time.Sleep(time.Second * time.Duration(60+rand.IntN(120)))
						}
					}
					if lastErr != nil {
						log.Printf("Failed to process textures: %+v", p.InputFiles)
						log.Printf("IngestTextures REST call failed with: %s", lastErr.Error())
					}

					processedTextureCount += len(p.InputFiles)
				})
		}
		batcher.Close()
		batcher = nil
		if _, err := httpClient.SaveLayer(activeLayer.LayerFilePath); err != nil {
			log.Fatalf("Was not able to save layer: %s", err.Error())
		}
	}
}
