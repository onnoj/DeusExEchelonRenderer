package internal

type (
	IngestTexturesRequestParams struct {
		InputHashes               []uint64
		InputFiles                []string
		InputTextureTypeStrings   []string
		InputTextureTypePathGlobs []string
		OutputDirectory           string
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

type IngestTexturesRequestParamsDataFlow struct {
	Name           string `json:"name"`
	PushInputData  bool   `json:"push_input_data,omitempty"`
	PushOutputData bool   `json:"push_output_data,omitempty"`
	Channel        string `json:"channel,omitempty"`
}

type IngestTexturesRequestParamsContextPluginData struct {
	ContextName              string                                `json:"context_name,omitempty"`
	InputFiles               [][]string                            `json:"input_files,omitempty"`
	OutputDirectory          string                                `json:"output_directory,omitempty"`
	AllowEmptyInputFilesList bool                                  `json:"allow_empty_input_files_list,omitempty"`
	DataFlows                []IngestTexturesRequestParamsDataFlow `json:"data_flows,omitempty"`
	HideContextUI            bool                                  `json:"hide_context_ui,omitempty"`
	CreateContextIfNotExist  bool                                  `json:"create_context_if_not_exist,omitempty"`
	ExposeMassUI             bool                                  `json:"expose_mass_ui,omitempty"`
	CookMassTemplate         bool                                  `json:"cook_mass_template,omitempty"`
	ExposeMassQueueActionUI  bool                                  `json:"expose_mass_queue_action_ui,omitempty"`
}

type IngestTexturesRequestParamsContextPlugin struct {
	Name string                                       `json:"name"`
	Data IngestTexturesRequestParamsContextPluginData `json:"data"`
}

type IngestTexturesRequestParamsSelectorPlugin struct {
	Name string      `json:"name"`
	Data interface{} `json:"data"`
}

type IngestTexturesRequestParamsResultorPluginData struct {
	Channel       string `json:"channel,omitempty"`
	CleanupOutput bool   `json:"cleanup_output,omitempty"`
}

type IngestTexturesRequestParamsResultorPlugin struct {
	Name string                                        `json:"name"`
	Data IngestTexturesRequestParamsResultorPluginData `json:"data"`
}

type IngestTexturesRequestParamsCheckPluginData struct {
	ShaderSubidentifiers    map[string]string                     `json:"shader_subidentifiers,omitempty"`
	DataFlows               []IngestTexturesRequestParamsDataFlow `json:"data_flows,omitempty"`
	ExposeMassQueueActionUI bool                                  `json:"expose_mass_queue_action_ui,omitempty"`
}

type IngestTexturesRequestParamsCheckPlugin struct {
	Name            string                                      `json:"name"`
	SelectorPlugins []IngestTexturesRequestParamsSelectorPlugin `json:"selector_plugins"`
	ResultorPlugins []IngestTexturesRequestParamsResultorPlugin `json:"resultor_plugins,omitempty"`
	Data            IngestTexturesRequestParamsCheckPluginData  `json:"data"`
	StopIfFixFailed bool                                        `json:"stop_if_fix_failed"`
	ContextPlugin   IngestTexturesRequestParamsContextPlugin    `json:"context_plugin"`
}

type IngestTexturesRequestParamsResultorPluginMain struct {
	Name string            `json:"name"`
	Data map[string]string `json:"data"`
}

type IngestTexturesRequestStruct struct {
	Executor        *int                                            `json:"executor,omitempty"`
	Name            *string                                         `json:"name,omitempty"`
	ContextPlugin   *IngestTexturesRequestParamsContextPlugin       `json:"context_plugin,omitempty"`
	CheckPlugins    []IngestTexturesRequestParamsCheckPlugin        `json:"check_plugins,omitempty"`
	ResultorPlugins []IngestTexturesRequestParamsResultorPluginMain `json:"resultor_plugins,omitempty"`
}

func NewIngestTexturesRequestParams() IngestTexturesRequestStruct {
	return IngestTexturesRequestStruct{
		ContextPlugin: &IngestTexturesRequestParamsContextPlugin{
			Name: "TextureImporter",
			Data: IngestTexturesRequestParamsContextPluginData{
				ContextName:              "ingestcraft",
				InputFiles:               [][]string{},
				OutputDirectory:          "",
				AllowEmptyInputFilesList: true,
				DataFlows: []IngestTexturesRequestParamsDataFlow{
					{Name: "InOutData", PushInputData: true},
				},
				HideContextUI:           true,
				CreateContextIfNotExist: true,
				ExposeMassUI:            true,
				CookMassTemplate:        true,
			},
		},
		CheckPlugins: []IngestTexturesRequestParamsCheckPlugin{
			{
				Name: "MaterialShaders",
				SelectorPlugins: []IngestTexturesRequestParamsSelectorPlugin{
					{
						Name: "AllMaterials",
						Data: struct{}{},
					},
				},
				Data: IngestTexturesRequestParamsCheckPluginData{
					ShaderSubidentifiers: map[string]string{"AperturePBR_Opacity": ".*"},
				},
				StopIfFixFailed: true,
				ContextPlugin: IngestTexturesRequestParamsContextPlugin{
					Name: "CurrentStage",
					Data: IngestTexturesRequestParamsContextPluginData{},
				},
			},
			{
				Name: "ConvertToOctahedral",
				SelectorPlugins: []IngestTexturesRequestParamsSelectorPlugin{
					{
						Name: "AllShaders",
						Data: struct{}{},
					},
				},
				ResultorPlugins: []IngestTexturesRequestParamsResultorPlugin{
					{
						Name: "FileCleanup",
						Data: IngestTexturesRequestParamsResultorPluginData{
							Channel:       "cleanup_files_normal",
							CleanupOutput: false,
						},
					},
				},
				Data: IngestTexturesRequestParamsCheckPluginData{
					DataFlows: []IngestTexturesRequestParamsDataFlow{
						{Name: "InOutData", PushInputData: true, PushOutputData: true, Channel: "cleanup_files_normal"},
					},
				},
				StopIfFixFailed: true,
				ContextPlugin: IngestTexturesRequestParamsContextPlugin{
					Name: "CurrentStage",
					Data: IngestTexturesRequestParamsContextPluginData{},
				},
			},
			{
				Name: "ConvertToDDS",
				SelectorPlugins: []IngestTexturesRequestParamsSelectorPlugin{
					{
						Name: "AllShaders",
						Data: struct{}{},
					},
				},
				ResultorPlugins: []IngestTexturesRequestParamsResultorPlugin{
					{
						Name: "FileCleanup",
						Data: IngestTexturesRequestParamsResultorPluginData{
							Channel:       "cleanup_files",
							CleanupOutput: false,
						},
					},
				},
				Data: IngestTexturesRequestParamsCheckPluginData{
					DataFlows: []IngestTexturesRequestParamsDataFlow{
						{Name: "InOutData", PushInputData: true, PushOutputData: true, Channel: "cleanup_files"},
						{Name: "InOutData", PushOutputData: true, Channel: "write_metadata"},
						{Name: "InOutData", PushOutputData: true, Channel: "ingestion_output"},
					},
				},
				StopIfFixFailed: true,
				ContextPlugin: IngestTexturesRequestParamsContextPlugin{
					Name: "CurrentStage",
					Data: IngestTexturesRequestParamsContextPluginData{},
				},
			},
			{
				Name: "MassTexturePreview",
				SelectorPlugins: []IngestTexturesRequestParamsSelectorPlugin{
					{
						Name: "Nothing",
						Data: struct{}{},
					},
				},
				Data: IngestTexturesRequestParamsCheckPluginData{
					ExposeMassQueueActionUI: true,
				},
				StopIfFixFailed: true,
				ContextPlugin: IngestTexturesRequestParamsContextPlugin{
					Name: "CurrentStage",
					Data: IngestTexturesRequestParamsContextPluginData{},
				},
			},
		},
		ResultorPlugins: []IngestTexturesRequestParamsResultorPluginMain{
			{
				Name: "FileMetadataWritter",
				Data: map[string]string{"channel": "write_metadata"},
			},
		},
	}
}
