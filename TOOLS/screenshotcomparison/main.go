package main

import (
	"fmt"
	"html/template"
	"image"
	"image/png"
	"log"
	"os"
	"path/filepath"
	"strings"
	"sync"

	"github.com/onnoj/DeusExEchelonRenderer/ScreenshotReportGenerator/util"
	_ "golang.org/x/image/bmp"
)

type (
	HTMLTemplateData struct {
		FileNames []string
		DataSets  []struct {
			Id        string
			HumanName string
		}
	}
)

var (
	GlobalTemplateData HTMLTemplateData
)

func main() {
	err := processFolders("./imagesets/")
	if err != nil {
		log.Fatalf("Error processing folders: %v", err)
	}

	err = generateHtml("index.template.html", "index.html")
	if err != nil {
		log.Fatalf("Error generating template: %v", err)
	}
}

func processFolders(root string) error {
	outerErr := error(nil)
	wg := sync.WaitGroup{}
	err := filepath.Walk(root, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		if info.IsDir() {
			if path != root {
				GlobalTemplateData.DataSets = append(GlobalTemplateData.DataSets, struct {
					Id        string
					HumanName string
				}{info.Name(), info.Name()})
			}
			return nil
		}

		if strings.ToLower(filepath.Ext(info.Name())) == ".bmp" {
			filename := info.Name()
			filename = filename[:len(filename)-len(filepath.Ext(filename))] + ".png"
			GlobalTemplateData.FileNames = util.AppendUnique(GlobalTemplateData.FileNames, filename)
		}

		if strings.ToLower(filepath.Ext(info.Name())) == ".png" {
			GlobalTemplateData.FileNames = util.AppendUnique(GlobalTemplateData.FileNames, info.Name())
		}

		if strings.ToLower(filepath.Ext(info.Name())) == ".bmp" {
			wg.Add(1)
			go func() {
				if err := convertBmpToPng(path); err != nil {
					log.Fatal(err.Error())
					return
				}
				wg.Done()
			}()
		}
		return nil
	})
	wg.Wait()

	if err != nil {
		return err
	} else {
		fmt.Println("OK")
	}

	return outerErr
}

func convertBmpToPng(bmpPath string) error {
	// Open the BMP file
	file, err := os.Open(bmpPath)
	if err != nil {
		return fmt.Errorf("failed to open BMP file %s: %v", bmpPath, err)
	}

	// Decode the BMP image
	img, _, err := image.Decode(file)
	if err != nil {
		return fmt.Errorf("failed to decode BMP file %s: %v", bmpPath, err)
	}
	file.Close()

	// Create the PNG file
	pngPath := bmpPath[:len(bmpPath)-len(filepath.Ext(bmpPath))] + ".png"
	pngFile, err := os.Create(pngPath)
	if err != nil {
		return fmt.Errorf("failed to create PNG file %s: %v", pngPath, err)
	}
	defer pngFile.Close()

	// Encode the image to PNG format
	if err := png.Encode(pngFile, img); err != nil {
		return fmt.Errorf("failed to encode PNG file %s: %v", pngPath, err)
	}

	// Remove the original BMP file
	if err := os.Remove(bmpPath); err != nil {
		return fmt.Errorf("failed to delete BMP file %s: %v", bmpPath, err)
	}

	fmt.Printf("Converted %s to %s and deleted the original BMP file.\n", bmpPath, pngPath)
	return nil
}

func generateHtml(templatePath string, outputPath string) error {
	//t, err := template.New("index.template.html").ParseFiles(templatePath)
	t, err := template.ParseFiles(templatePath)
	if err != nil {
		return err
	}

	f, err := os.OpenFile(outputPath, os.O_CREATE|os.O_TRUNC, 0777)
	if err != nil {
		return err
	}

	err = t.Execute(f, GlobalTemplateData)
	return err
}
