package internal

import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
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

type (
	IngestTexturesRequestParams struct {
		InputFiles []struct {
			InputFilePath string
			TextureType   string
		}
		OutputDirectory string
	}

	// IngestTexturesResponse structure
	IngestTexturesResponse struct {
		CompletedSchemas []IngestTexturesResponseCompletedSchema `json:"completed_schemas,omitempty"`
	}

	// IngestTexturesResponseCompletedSchema structure
	IngestTexturesResponseCompletedSchema struct {
		Name             string                                 `json:"name,omitempty"`
		UUID             string                                 `json:"uuid,omitempty"`
		Data             IngestTexturesResponseData             `json:"data,omitempty"`
		Progress         float64                                `json:"progress,omitempty"`
		SendRequest      bool                                   `json:"send_request,omitempty"`
		ContextPlugin    IngestTexturesResponseContextPlugin    `json:"context_plugin,omitempty"`
		CheckPlugins     []IngestTexturesResponseCheckPlugin    `json:"check_plugins,omitempty"`
		ResultorPlugins  []IngestTexturesResponseResultorPlugin `json:"resultor_plugins,omitempty"`
		ValidationPassed bool                                   `json:"validation_passed,omitempty"`
		Finished         []interface{}                          `json:"finished,omitempty"`
	}

	// IngestTexturesResponseData structure
	IngestTexturesResponseData struct {
		NameTooltip                    string                           `json:"name_tooltip,omitempty"`
		Channel                        string                           `json:"channel,omitempty"`
		ExposeMassUI                   bool                             `json:"expose_mass_ui,omitempty"`
		ExposeMassQueueActionUI        bool                             `json:"expose_mass_queue_action_ui,omitempty"`
		CookMassTemplate               bool                             `json:"cook_mass_template,omitempty"`
		DisplayNameMassTemplate        string                           `json:"display_name_mass_template,omitempty"`
		DisplayNameMassTemplateTooltip string                           `json:"display_name_mass_template_tooltip,omitempty"`
		UUID                           string                           `json:"uuid,omitempty"`
		Progress                       []interface{}                    `json:"progress,omitempty"`
		GlobalProgressValue            float64                          `json:"global_progress_value,omitempty"`
		LastCheckMessage               string                           `json:"last_check_message,omitempty"`
		LastCheckTiming                float64                          `json:"last_check_timing,omitempty"`
		LastCheckResult                bool                             `json:"last_check_result,omitempty"`
		LastSetMessage                 *string                          `json:"last_set_message,omitempty"`
		LastSetTiming                  *float64                         `json:"last_set_timing,omitempty"`
		LastSetResult                  *bool                            `json:"last_set_result,omitempty"`
		LastOnExitMessage              *string                          `json:"last_on_exit_message,omitempty"`
		LastOnExitTiming               *float64                         `json:"last_on_exit_timing,omitempty"`
		LastOnExitResult               *bool                            `json:"last_on_exit_result,omitempty"`
		HideContextUI                  bool                             `json:"hide_context_ui,omitempty"`
		ContextName                    string                           `json:"context_name,omitempty"`
		CreateContextIfNotExist        bool                             `json:"create_context_if_not_exist,omitempty"`
		ComputedContext                string                           `json:"computed_context,omitempty"`
		AllowEmptyInputFilesList       bool                             `json:"allow_empty_input_files_list,omitempty"`
		InputFiles                     [][]string                       `json:"input_files,omitempty"`
		ErrorOnTextureTypes            *string                          `json:"error_on_texture_types,omitempty"`
		CreateOutputDirectoryIfMissing bool                             `json:"create_output_directory_if_missing,omitempty"`
		OutputDirectory                string                           `json:"output_directory,omitempty"`
		DataFlows                      []IngestTexturesResponseDataFlow `json:"data_flows,omitempty"`
	}

	// IngestTexturesResponseDataFlow structure
	IngestTexturesResponseDataFlow struct {
		Name           string      `json:"name,omitempty"`
		Channel        string      `json:"channel,omitempty"`
		InputData      []string    `json:"input_data,omitempty"`
		PushInputData  bool        `json:"push_input_data,omitempty"`
		OutputData     interface{} `json:"output_data,omitempty"`
		PushOutputData bool        `json:"push_output_data,omitempty"`
	}

	// IngestTexturesResponseContextPlugin structure
	IngestTexturesResponseContextPlugin struct {
		Name            string                     `json:"name,omitempty"`
		Enabled         bool                       `json:"enabled,omitempty"`
		Data            IngestTexturesResponseData `json:"data,omitempty"`
		ResultorPlugins interface{}                `json:"resultor_plugins,omitempty"`
	}

	// IngestTexturesResponseCheckPlugin structure
	IngestTexturesResponseCheckPlugin struct {
		Name             string                                 `json:"name,omitempty"`
		Enabled          bool                                   `json:"enabled,omitempty"`
		Data             IngestTexturesResponseData             `json:"data,omitempty"`
		ContextPlugin    IngestTexturesResponseContextPlugin    `json:"context_plugin,omitempty"`
		SelectorPlugins  []IngestTexturesResponseSelectorPlugin `json:"selector_plugins,omitempty"`
		ResultorPlugins  interface{}                            `json:"resultor_plugins,omitempty"`
		StopIfFixFailed  bool                                   `json:"stop_if_fix_failed,omitempty"`
		PauseIfFixFailed bool                                   `json:"pause_if_fix_failed,omitempty"`
	}

	// IngestTexturesResponseSelectorPlugin structure
	IngestTexturesResponseSelectorPlugin struct {
		Name    string                     `json:"name,omitempty"`
		Enabled bool                       `json:"enabled,omitempty"`
		Data    IngestTexturesResponseData `json:"data,omitempty"`
	}

	// IngestTexturesResponseResultorPlugin structure
	IngestTexturesResponseResultorPlugin struct {
		Name    string                     `json:"name,omitempty"`
		Enabled bool                       `json:"enabled,omitempty"`
		Data    IngestTexturesResponseData `json:"data,omitempty"`
	}
)

func (c *RestClient) IngestTextures(params IngestTexturesRequestParams) (IngestTexturesResponse, error) {
	jsonMap := make(map[string]any)
	contextPluginJson := make(map[string]any)
	dataJson := make(map[string]any)

	inputFiles := make([][]string, 0)
	for _, input := range params.InputFiles {
		inputFiles = append(inputFiles, []string{input.InputFilePath, input.TextureType})
	}
	dataJson["input_files"] = inputFiles
	dataJson["output_directory"] = params.OutputDirectory

	contextPluginJson["data"] = dataJson
	jsonMap["context_plugin"] = contextPluginJson
	jsonResponse, err := c.Post("/ingestcraft/mass-validator/queue/material", jsonMap)
	if err != nil {
		return IngestTexturesResponse{}, err
	}
	os.WriteFile("d:\\temp\\mongoose.txt", []byte(jsonResponse), 0777)

	var obj IngestTexturesResponse
	if err := json.Unmarshal([]byte(jsonResponse), &obj); err != nil {
		return IngestTexturesResponse{}, err
	}

	return obj, nil
}

//func (c *RestClient) PutOverrideTextures()
