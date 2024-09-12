package internal

import (
	"errors"
	"fmt"
	"net/url"
)

type (
	Layer struct {
		LayerFilePath string  `json:"layer_id"`
		LayerType     string  `json:"layer_type"`
		Children      []Layer `json:"children"`
	}

	GetLayersResponse struct {
		Layers []Layer `json:"layers"`
	}
)

type LayerType string

const (
	LayerType_AutoUpscale  = LayerType("autoupscale")
	LayerType_CaptureBaker = LayerType("capture_baker")
	LayerType_Capture      = LayerType("capture")
	LayerType_Replacement  = LayerType("replacement")
	LayerType_Workfile     = LayerType("workfile")
)

func (c *RestClient) GetLayers(layerTypes []string, layerCount int) (GetLayersResponse, error) {
	params := make(map[string][]string)
	if len(layerTypes) > 0 {
		params["layer_types"] = append(params["layer_types"], layerTypes...)
	}
	params["layer_count"] = append(params["layer_count"], fmt.Sprint(layerCount))

	l, err := TypedGet[GetLayersResponse](c, "/stagecraft/layers/", &params)
	if err != nil {
		return GetLayersResponse{}, err
	}

	if l != nil {
		return *l, nil
	}

	return GetLayersResponse{}, errors.New("unknown response received")
}

func (c *RestClient) GetEditLayer() (string, error) {
	s, err := c.Get("/stagecraft/layers/target", nil)
	if err != nil {
		return "", err
	}

	return s, nil
}

func (c *RestClient) SetEditLayer(layer_id string) (bool, error) {
	s, err := c.Put(fmt.Sprintf("/stagecraft/layers/target/%s", url.QueryEscape(layer_id)), nil)
	if err != nil {
		return false, err
	}

	if s == "OK" {
		return true, nil
	}

	return false, nil
}

type (
	PostCreateLayerRequestParams struct {
		LayerPath        string
		LayerType        LayerType
		SetEditTarget    *bool
		SublayerPosition *int
		ParentLayerID    *string
		CreateOrInsert   *bool
		ReplaceExisting  *bool
	}
)

func (c *RestClient) PostCreateLayer(params PostCreateLayerRequestParams) (string, error) {
	jsonMap := make(map[string]any)

	jsonMap["layer_path"] = params.LayerPath
	jsonMap["layer_type"] = params.LayerType
	if params.SetEditTarget != nil {
		jsonMap["set_edit_target"] = *params.SetEditTarget
	}
	if params.SublayerPosition != nil {
		jsonMap["sublayer_position"] = *params.SublayerPosition
	}
	if params.ParentLayerID != nil {
		jsonMap["parent_layer_id"] = *params.ParentLayerID
	}
	if params.CreateOrInsert != nil {
		jsonMap["create_or_insert"] = *params.CreateOrInsert
	}
	if params.ReplaceExisting != nil {
		jsonMap["replace_existing"] = *params.ReplaceExisting
	}

	resp, err := c.Post("/stagecraft/layers/", jsonMap)
	if err != nil {
		return "", err
	}

	return resp, nil
}

func (c *RestClient) SaveLayer(layer_id string) (bool, error) {
	s, err := c.Post(fmt.Sprintf("/stagecraft/layers/%s/save", url.QueryEscape(layer_id)), nil)
	if err != nil {
		return false, err
	}

	if s == "OK" {
		return true, nil
	}

	return false, nil
}

//func (c *RestClient) PutSetSelection()
// /stagecraft/assets/selection/{asset_paths}
// asset_paths: Comma-separated list of asset paths to select
