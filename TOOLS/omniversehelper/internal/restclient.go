package internal

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"net/http"
	"strings"
	"time"

	"github.com/onnoj/DeusExEchelonRenderer/OmniverseHelper/utils"
)

// RestClient is a basic HTTP client
type RestClient struct {
	BaseURL string
	Timeout time.Duration
}

// NewClient creates a new RestClient instance
func NewClient(baseURL string, timeout time.Duration) *RestClient {
	return &RestClient{
		BaseURL: baseURL,
		Timeout: timeout,
	}
}

func TypedGet[ResponseType any](c *RestClient, endpoint string, parameters *map[string][]string) (*ResponseType, error) {
	body, err := c.Get(endpoint, parameters)
	if err != nil {
		return nil, err
	}

	if body == "" {
		return nil, errors.New("empty body was returned")
	}

	var obj ResponseType
	if err := json.Unmarshal([]byte(body), &obj); err != nil {
		return nil, err
	}
	return &obj, nil
}

// Get performs a GET request to the specified path
func (c *RestClient) Get(endpoint string, parameters *map[string][]string) (string, error) {
	client := http.Client{
		Timeout: c.Timeout,
	}
	url := fmt.Sprintf("%s%s", c.BaseURL, endpoint)

	if parameters != nil && len(*parameters) > 0 {
		url += "?"
		for key, valueSlice := range *parameters {
			for _, v := range valueSlice {
				url += fmt.Sprintf("%s=%s&", key, v)
			}
		}
		url, _ = strings.CutSuffix(url, "&")
	}

	resp, err := client.Get(url)
	if err != nil {
		return "", fmt.Errorf("failed to GET from %s: %v", url, err)
	}
	defer resp.Body.Close()

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return "", fmt.Errorf("failed to read response body: %v", err)
	}

	if resp.StatusCode != http.StatusOK {
		return "", fmt.Errorf("received non-200 response code: %d", resp.StatusCode)
	}

	return string(body), nil
}

func (c *RestClient) Post(endpoint string, jsonMap any) (string, error) {
	client := http.Client{
		Timeout: c.Timeout,
	}
	url := fmt.Sprintf("%s%s", c.BaseURL, endpoint)

	obj, err := json.Marshal(jsonMap)
	if err != nil {
		return "", err
	}

	resp, err := client.Post(url, "application/json", bytes.NewBuffer(obj))
	if err != nil {
		return "", fmt.Errorf("failed to POST to %s: %v", url, err)
	}
	defer resp.Body.Close()

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return "", fmt.Errorf("error reading response body: %v", err)
	}

	// Optionally check for the status code
	if resp.StatusCode != http.StatusOK && resp.StatusCode != http.StatusCreated {
		return "", fmt.Errorf("unexpected status code: %d, response: %s", resp.StatusCode, string(body))
	}

	return string(body), nil
}

func (c *RestClient) Put(endpoint string, jsonMap any) (string, error) {
	client := http.Client{
		Timeout: c.Timeout,
	}
	url := fmt.Sprintf("%s%s", c.BaseURL, endpoint)

	obj, err := json.Marshal(jsonMap)
	if err != nil {
		return "", err
	}

	dbg := utils.NewDebugWriter(endpoint, ".json")
	defer dbg.Close()

	dbg.Write("=========================== Request ====================\n")
	dbg.Write(url + "\n")
	dbg.Write("=========================== Request ====================\n")
	dbg.Write(string(obj))
	req, err := http.NewRequest(http.MethodPut, url, bytes.NewBuffer(obj))
	if err != nil {
		return "", fmt.Errorf("failed to PUT to %s: %v", url, err)
	}
	req.Header.Set("Content-Type", "application/json")
	resp, err := client.Do(req)
	if err != nil || resp == nil {
		return "", errors.New("unknown error")
	}
	defer resp.Body.Close()

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return "", fmt.Errorf("error reading response body: %v", err)
	}
	dbg.Write("=========================== Answer ====================\n")
	dbg.Write(string(body) + "\n")
	dbg.Write("=========================== Answer ====================\n")

	// Optionally check for the status code
	if resp.StatusCode != http.StatusOK {
		return "", fmt.Errorf("unexpected status code: %d, response: %s", resp.StatusCode, string(body))
	}

	return string(body), nil
}
