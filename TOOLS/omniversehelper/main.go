package main

import (
	"fmt"
	"log"
	"os"
	"regexp"
	"strconv"
	"strings"

	"github.com/onnoj/DeusExEchelonRenderer/OmniverseHelper/utils"
	"github.com/spf13/cobra"
)

var (
	RootCmdOptions struct {
		InputFolder     string
		TextureDumpJSON string
	}

	IngestCmdOptions struct {
		OverwriteOverrides    bool
		InputFolderRecursive  bool
		PackageFilters        []string
		PackageFiltersRegexes []*regexp.Regexp
	}

	CopyCmdOptions struct {
		OutputFolderPath    string
		InputFilenameFormat string
		OutputFormat        string
		JsonFilters         []string
		JsonFilterMap       map[string]*regexp.Regexp
		//JsonFiltersRegexes       []*regexp.Regexp
		InputFileFilters        []string
		InputFileFiltersRegexes []*regexp.Regexp
		RequireEqualWidthHeight bool
		RejectEqualWidthHeight  bool
		RequireMinWidth         uint64
		RequireMinHeight        uint64
		RejectMinWidth          uint64
		RejectMinHeight         uint64
	}
)

// rootCmd represents the base command when called without any subcommands
var rootCmd = &cobra.Command{
	Use:   "omniversehelper",
	Short: "Omniverse Helper CLI",
	Long:  `Omniverse Helper CLI is a tool for interacting with various REST APIs and performing useful tasks.`,
	Args:  cobra.MinimumNArgs(1),
	PersistentPreRun: func(cmd *cobra.Command, args []string) {
		RootCmdOptions.InputFolder = utils.Unquote(RootCmdOptions.InputFolder)
		RootCmdOptions.TextureDumpJSON = utils.Unquote(RootCmdOptions.TextureDumpJSON)
	},
	Run: func(cmd *cobra.Command, args []string) {
		fmt.Println("Omniverse Helper CLI")
	},
}

func init() {
	rootCmd.PersistentFlags().StringVarP(&RootCmdOptions.InputFolder, "input", "i", "", "--input=<folder>")
	rootCmd.MarkFlagRequired("input")

	rootCmd.PersistentFlags().StringVarP(&RootCmdOptions.TextureDumpJSON, "jsondump", "j", "", "--jsondump=<path/to/.json>")
	rootCmd.MarkFlagRequired("jsondump")

	ingestCommand := cobra.Command{
		Use:   "ingest",
		Args:  cobra.NoArgs,
		Short: "Mass ingests and assigns ingested textures",
		Long:  "Mass ingests and assigns ingested textures to RTX Remix, requires the toolkit to be running with an active project loaded.",
		PersistentPreRun: func(cmd *cobra.Command, args []string) {
			IngestCmdOptions.PackageFilters = utils.Unquote(IngestCmdOptions.PackageFilters)
		},
		Run: func(cmd *cobra.Command, args []string) {
			for _, s := range IngestCmdOptions.PackageFilters {
				IngestCmdOptions.PackageFiltersRegexes = append(IngestCmdOptions.PackageFiltersRegexes, regexp.MustCompile(s))
			}
			ExecuteMassIngest()
		},
	}
	ingestCommand.Flags().BoolVar(&IngestCmdOptions.OverwriteOverrides, "overwrite-overrides", false, "--overwrite-overrides, can be specified to always set a material override even if one already exists.")
	ingestCommand.Flags().BoolVarP(&IngestCmdOptions.InputFolderRecursive, "recursive", "R", false, "--recursive")
	ingestCommand.Flags().StringArrayVar(&IngestCmdOptions.PackageFilters, "package-filter", []string{}, "--package-filter=<regex>, can be specified multiple times.")
	rootCmd.AddCommand(&ingestCommand)

	copyCommand := cobra.Command{
		Use:   "copy",
		Args:  cobra.NoArgs,
		Short: "Conditionally copy files in the given folder",
		Long:  "Conditionally copy files in the given folder, and applies some basic name processing",
		PersistentPreRun: func(cmd *cobra.Command, args []string) {
			CopyCmdOptions.OutputFolderPath = utils.Unquote(CopyCmdOptions.OutputFolderPath)
			CopyCmdOptions.InputFilenameFormat = utils.Unquote(CopyCmdOptions.InputFilenameFormat)
			CopyCmdOptions.OutputFormat = utils.Unquote(CopyCmdOptions.OutputFormat)
			CopyCmdOptions.JsonFilters = utils.Unquote(CopyCmdOptions.JsonFilters)
			CopyCmdOptions.InputFileFilters = utils.Unquote(CopyCmdOptions.InputFileFilters)
		},
		Run: func(cmd *cobra.Command, args []string) {
			CopyCmdOptions.JsonFilterMap = make(map[string]*regexp.Regexp)
			for _, s := range CopyCmdOptions.JsonFilters {
				if unquoted, err := strconv.Unquote(s); err == nil {
					s = unquoted
				}

				if strings.Contains(s, "=") {
					p := strings.Split(s, "=")
					CopyCmdOptions.JsonFilterMap[p[0]] = regexp.MustCompile(p[1])
				} else {
					log.Fatal("JsonFilter was not properly formatted, should be key=value")
				}
			}
			for _, s := range CopyCmdOptions.InputFileFilters {
				CopyCmdOptions.InputFileFiltersRegexes = append(CopyCmdOptions.InputFileFiltersRegexes, regexp.MustCompile(s))
			}
			dir := RootCmdOptions.InputFolder
			if s, e := os.Stat(dir); e == nil && s.IsDir() {
				ExecuteCopy(dir)
			} else {
				fmt.Printf("Error: %s is not a directory\n", dir)
			}
		},
	}
	copyCommand.Flags().StringVarP(&CopyCmdOptions.OutputFolderPath, "output", "o", "", "Path to the folder that will contain all the output")
	copyCommand.Flags().StringVar(&CopyCmdOptions.InputFilenameFormat, "inputfilenameformat", "%X", "The format string to try parse the ID from the filename")
	copyCommand.Flags().StringVar(&CopyCmdOptions.OutputFormat, "outputformat", "${package}\\${filename}", "The format for the output file operation, sourced directly from the dump .json")
	copyCommand.Flags().StringArrayVar(&CopyCmdOptions.JsonFilters, "jsonfilter", []string{}, "Match against specific key/value pairs in the JSON. ie, 'path=.*Glass.*'")
	copyCommand.Flags().StringArrayVar(&CopyCmdOptions.InputFileFilters, "inputfilter", []string{}, "Match against specific files, ie, '.*_diffuse.png'")
	copyCommand.Flags().BoolVar(&CopyCmdOptions.RequireEqualWidthHeight, "require-equal-widthheight", false, "Only copy textures that have an equal width and height")
	copyCommand.Flags().BoolVar(&CopyCmdOptions.RejectEqualWidthHeight, "reject-equal-widthheight", false, "Only copy textures that have a mismatching width and height")
	copyCommand.Flags().Uint64Var(&CopyCmdOptions.RequireMinWidth, "require-min-width", 0, "Skip textures that are below this width")
	copyCommand.Flags().Uint64Var(&CopyCmdOptions.RequireMinHeight, "require-min-height", 0, "Skip textures that are below this height")
	copyCommand.Flags().Uint64Var(&CopyCmdOptions.RejectMinWidth, "reject-min-width", 0, "Skip textures that are above this width are")
	copyCommand.Flags().Uint64Var(&CopyCmdOptions.RejectMinHeight, "reject-min-height", 0, "Skip textures that are above this height are")
	rootCmd.AddCommand(&copyCommand)
}

func main() {
	cobra.EnableTraverseRunHooks = true
	if err := rootCmd.Execute(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}
