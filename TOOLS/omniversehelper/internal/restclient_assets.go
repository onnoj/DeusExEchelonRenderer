package internal

import (
	"encoding/json"
	"errors"
	"fmt"
)

type (
	GetAssetsQueryParams struct {
		AssetHashes         []string
		AssetTypes          []string
		Selection           *bool
		FilterSessionAssets *bool
		LayerIdentifier     *string
		Exists              *bool
	}
)

const ()

func (c *RestClient) GetAssetsDefaultDirectory() (string, error) {
	s, err := c.Get("/stagecraft/assets/default-directory", nil)
	if err != nil {
		return "", err
	}

	jsonObj := make(map[string]string)
	if err := json.Unmarshal([]byte(s), &jsonObj); err != nil {
		return "", err
	}

	if p, found := jsonObj["asset_path"]; found {
		return p, nil
	}

	return "", errors.New("unexpected error, asset_path was not found in response")
}

func (c *RestClient) GetAssets(query GetAssetsQueryParams) ([]string, error) {
	params := make(map[string][]string)
	params["asset_hashes"] = append(params["asset_hashes"], query.AssetHashes...)
	params["asset_types"] = append(params["asset_types"], query.AssetTypes...)

	if query.Selection != nil {
		params["selection"] = append(params["selection"], fmt.Sprint(*query.Selection))
	}

	if query.FilterSessionAssets != nil {
		params["filter_session_assets"] = append(params["filter_session_assets"], fmt.Sprint(*query.FilterSessionAssets))
	}

	if query.LayerIdentifier != nil {
		params["layer_identifier"] = append(params["layer_identifier"], fmt.Sprint(*query.LayerIdentifier))
	}

	if query.Selection != nil {
		params["exists"] = append(params["exists"], fmt.Sprint(*query.Exists))
	}

	s, err := c.Get("/stagecraft/assets/", &params)
	if err != nil {
		return nil, err
	}

	jsonObj := make(map[string][]string)
	if err := json.Unmarshal([]byte(s), &jsonObj); err != nil {
		return nil, err
	}

	if p, found := jsonObj["asset_paths"]; found {
		return p, nil
	}

	return nil, errors.New("could not parse response")
}
