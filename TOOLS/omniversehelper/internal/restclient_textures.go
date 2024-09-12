package internal

import (
	"encoding/json"
	"errors"
	"fmt"
	"log"
	"net/url"
	"os"

	"github.com/onnoj/DeusExEchelonRenderer/OmniverseHelper/utils"
)

type (
	GetTexturesRequestParams struct {
		Selection          *bool
		FilterSessionPrims *bool
		Exists             *bool
		AssetHashes        []string
		TextureTypes       []string
	}

	GetTexturesResponse struct {
		Textures [][2]string `json:"textures"`
	}
)

func (c *RestClient) GetTextures(params GetTexturesRequestParams) (GetTexturesResponse, error) {
	obj := make(map[string][]string)

	if params.Selection != nil {
		obj["selection"] = append(obj["selection"], fmt.Sprint(*params.Selection))
	}

	if params.FilterSessionPrims != nil {
		obj["filter_session_prims"] = append(obj["filter_session_prims"], fmt.Sprint(*params.FilterSessionPrims))
	}

	if params.Exists != nil {
		obj["exists"] = append(obj["exists"], fmt.Sprint(*params.Exists))
	}

	if len(params.AssetHashes) > 0 {
		obj["asset_hashes"] = append(obj["asset_hashes"], params.AssetHashes...)
	}

	if len(params.TextureTypes) > 0 {
		obj["textureTypes"] = append(obj["textureTypes"], params.TextureTypes...)
	}

	l, err := TypedGet[GetTexturesResponse](c, "/stagecraft/textures/", &obj)
	if err != nil {
		return GetTexturesResponse{}, err
	}

	if l != nil {
		return *l, nil
	}

	return GetTexturesResponse{}, errors.New("unknown response received")
}

func (c *RestClient) IngestTextures(params IngestTexturesRequestParams) (IngestTexturesResponse, error) {

	inputFiles := make([][]string, 0)
	for i := int(0); i < len(params.InputFiles); i++ {
		inputFiles = append(inputFiles, []string{params.InputFiles[i], params.InputTextureTypeStrings[i]})
	}

	requestParams := NewIngestTexturesRequestParams()
	requestParams.CheckPlugins = nil    //[]IngestTexturesRequestParamsCheckPlugin{}
	requestParams.ResultorPlugins = nil //[]IngestTexturesRequestParamsResultorPluginMain{}
	requestParams.Executor = utils.NewPtrValue(0)
	requestParams.Name = nil

	requestData := &requestParams.ContextPlugin.Data
	requestData.InputFiles = inputFiles
	requestData.OutputDirectory = params.OutputDirectory

	jsonResponse, err := c.Post("/ingestcraft/mass-validator/queue/material", requestParams)
	if err != nil {
		return IngestTexturesResponse{}, err
	}

	f, _ := os.CreateTemp("", "ingesttextures_*.json")
	js, _ := json.Marshal(requestParams)
	f.WriteString("===================== REQ ==================\n")
	f.WriteString(string(js) + "\n")
	f.WriteString("===================== RESP ==================\n")
	f.WriteString(jsonResponse + "\n")
	defer f.Close()

	/*
			jsonMap := make(map[string]any)
		contextPluginJson := make(map[string]any)
		dataJson := make(map[string]any)

			dataJson["input_files"] = inputFiles
			dataJson["output_directory"] = params.OutputDirectory

			contextPluginJson["data"] = dataJson
			jsonMap["context_plugin"] = contextPluginJson
			jsonMap["check_plugins"]
			jsonResponse, err := c.Post("/ingestcraft/mass-validator/queue/material", jsonMap)
			if err != nil {
				return IngestTexturesResponse{}, err
			}

	*/
	//if len(params.InputFiles) > 0 {
	//	os.WriteFile("d:\\temp\\mongoose.txt", []byte(jsonResponse), 0777)
	//	panic(1)
	//}

	var obj IngestTexturesResponse
	if err := json.Unmarshal([]byte(jsonResponse), &obj); err != nil {
		return IngestTexturesResponse{}, err
	}

	return obj, nil
}

func (c *RestClient) ValidateTextureMaterialInputs(texturePath string) ([]string, error) {
	type response struct {
		AssetPaths []string `json:"asset_paths"`
	}

	endpoint := fmt.Sprintf("/stagecraft/textures/%s/material/inputs", url.QueryEscape(texturePath))
	resp, err := TypedGet[response](c, endpoint, nil)
	if err != nil {
		return nil, err
	}
	if resp == nil || len(resp.AssetPaths) == 0 {
		return nil, errors.New("empty response was returned")
	}

	return resp.AssetPaths, nil
}

func (c *RestClient) SetOverrideTextures(texturePaths []string, filePaths []string, force bool) (bool, error) {
	if len(texturePaths) != len(filePaths) {
		log.Fatal("Number of texturePaths and filePaths need to be the same; it's a 1:1 mapping!")
	}

	body := struct {
		Force    bool       `json:"force,omitempty"`
		Textures [][]string `json:"textures,omitempty"`
	}{
		Force: force,
	}
	for i := 0; i < len(texturePaths); i++ {
		body.Textures = append(body.Textures, []string{texturePaths[i], filePaths[i]})
	}

	s, err := c.Put("/stagecraft/textures/", &body)
	if err != nil {
		return false, err
	}

	if s == "OK" {
		return true, nil
	}

	return false, nil
}
