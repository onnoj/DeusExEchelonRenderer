package main

import (
	"fmt"
	"log"
	"os"
	"regexp"
	"strings"

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
	}
)

// rootCmd represents the base command when called without any subcommands
var rootCmd = &cobra.Command{
	Use:   "omniversehelper",
	Short: "Omniverse Helper CLI",
	Long:  `Omniverse Helper CLI is a tool for interacting with various REST APIs and performing useful tasks.`,
	Args:  cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		fmt.Println("Omniverse Helper CLI")
	},
}

func init() {
	rootCmd.PersistentFlags().StringVarP(&RootCmdOptions.InputFolder, "input", "i", "", "--input=<folder>")
	rootCmd.PersistentFlags().StringVarP(&RootCmdOptions.TextureDumpJSON, "jsondump", "j", "", "--jsondump=<path/to/.json>")

	ingestCommand := cobra.Command{
		Use:   "ingest",
		Args:  cobra.NoArgs,
		Short: "Mass ingests and assigns ingested textures",
		Long:  "Mass ingests and assigns ingested textures to RTX Remix, requires the toolkit to be running with an active project loaded.",
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
		Run: func(cmd *cobra.Command, args []string) {
			for _, s := range CopyCmdOptions.JsonFilters {
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
	rootCmd.AddCommand(&copyCommand)
}

func main() {
	if err := rootCmd.Execute(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}
