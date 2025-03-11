package main

import (
	"encoding/json"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"reflect"
	"regexp"
	"strings"

	"github.com/onnoj/DeusExEchelonRenderer/OmniverseHelper/data"
	. "github.com/onnoj/DeusExEchelonRenderer/OmniverseHelper/internal"
	"github.com/onnoj/DeusExEchelonRenderer/OmniverseHelper/utils"
)

// Helper function to copy files
func copyFile(src, dst string) error {
	sourceFileStat, err := os.Stat(src)
	if err != nil {
		return err
	}

	if !sourceFileStat.Mode().IsRegular() {
		return fmt.Errorf("%s is not a regular file", src)
	}

	source, err := os.Open(src)
	if err != nil {
		return err
	}
	defer source.Close()

	destination, err := os.Create(dst)
	if err != nil {
		return err
	}
	defer destination.Close()

	_, err = io.Copy(destination, source)
	return err
}

func ExecuteCopy(inputDirectory string) {
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

	sanitizeName := func(pName string) string {
		replacements := []struct {
			old, new string
		}{
			{"-", "_"}, {"/", "_"}, {"<", "_"}, {">", "_"},
			{":", "_"}, {"\"", "_"}, {"\\", "_"}, {"|", "_"},
			{"?", "_"}, {"*", "_"}, {" ", "_"}, {".", "_"},
			{"\t", "_"},
		}

		for _, pair := range replacements {
			pName = strings.ReplaceAll(pName, pair.old, pair.new)
		}
		return pName
	}

	files, err := os.ReadDir(RootCmdOptions.InputFolder)
	if err != nil {
		log.Fatal(err)
	}

	for _, file := range files {
		if file.IsDir() {
			continue
		}

		filePath := filepath.Join(RootCmdOptions.InputFolder, file.Name())

		// Apply input file filters if provided
		if len(CopyCmdOptions.InputFileFiltersRegexes) > 0 {
			hasMatch := false
			for _, r := range CopyCmdOptions.InputFileFiltersRegexes {
				if r.MatchString(filePath) {
					hasMatch = true
					break
				}
			}
			if !hasMatch {
				continue
			}
		}

		// Match fileHash from filename:
		var fileHash uint64 = 0
		if _, err := fmt.Sscanf(file.Name(), CopyCmdOptions.InputFilenameFormat, &fileHash); err != nil {
			fmt.Println("File does not match format:", file.Name())
			continue
		}

		hasJsonFilters := len(CopyCmdOptions.JsonFilterMap) > 0
		if data, ok := TextureMapping[fileHash]; ok {
			hasMatch := false
			fileNameCandidate := CopyCmdOptions.OutputFormat

			for _, field := range reflect.VisibleFields(reflect.TypeOf(*data)) {
				var variableName string
				if strings.Contains(string(field.Tag), "json:") {
					re := regexp.MustCompile(`json:"([^,"]*)`)
					variableName = re.FindStringSubmatch(string(field.Tag))[1]
				} else {
					variableName = field.Name
				}

				variable := fmt.Sprintf("${%s}", variableName)
				replacementValue := fmt.Sprintf("%v", reflect.ValueOf(*data).FieldByIndex(field.Index))

				if strings.HasPrefix(replacementValue, "\"") && strings.HasSuffix(replacementValue, "\"") {
					replacementValue = replacementValue[1 : len(replacementValue)-1]
				}

				if regex, exists := CopyCmdOptions.JsonFilterMap[variableName]; exists {
					hasMatch = regex.MatchString(replacementValue) || hasMatch
				}

				replacementValue = sanitizeName(replacementValue)
				fileNameCandidate = strings.ReplaceAll(fileNameCandidate, variable, replacementValue)
			}

			fileNameCandidate = strings.ReplaceAll(fileNameCandidate, "${filename}", file.Name())

			if hasJsonFilters && !hasMatch {
				continue
			}

			textureOutputFile := filepath.Join(CopyCmdOptions.OutputFolderPath, fileNameCandidate)
			textureOutputFolder := filepath.Dir(textureOutputFile)

			if _, err := os.Stat(textureOutputFolder); os.IsNotExist(err) {
				err := os.MkdirAll(textureOutputFolder, os.ModePerm)
				if err != nil {
					log.Fatalf("Unable to create path: %s", textureOutputFolder)
				}
			}

			inputFilePath := filepath.Join(RootCmdOptions.InputFolder, file.Name())
			err := copyFile(inputFilePath, textureOutputFile)
			if err == nil {
				log.Printf("Copied %s\t\t => \t\t %s\n", inputFilePath, textureOutputFile)
			} else {
				log.Fatalf("Failed to copy file: %v", err)
			}
		}
	}
}
