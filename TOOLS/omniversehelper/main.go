package main

import (
	"fmt"
	"os"
	"regexp"

	"github.com/spf13/cobra"
)

var (
	Options struct {
		TextureDumpJSON      string
		InputFolder          string
		InputFolderRecursive bool

		PackageFilters        []string
		PackageFiltersRegexes []*regexp.Regexp
	}
)

// rootCmd represents the base command when called without any subcommands
var rootCmd = &cobra.Command{
	Use:   "omniversehelper",
	Short: "Omniverse Helper CLI",
	Long:  `Omniverse Helper CLI is a tool for interacting with various REST APIs and performing useful tasks.`,
	Run: func(cmd *cobra.Command, args []string) {
		fmt.Println("Omniverse Helper CLI")
	},
}

func init() {
	rootCmd.Flags().StringVarP(&Options.InputFolder, "input", "i", "", "--input=<folder>")
	rootCmd.Flags().BoolVarP(&Options.InputFolderRecursive, "recursive", "R", false, "--recursive")
	rootCmd.Flags().StringVarP(&Options.TextureDumpJSON, "jsondump", "j", "", "--jsondump=<path/to/.json>")
	rootCmd.Flags().StringArrayVar(&Options.PackageFilters, "package-filter", []string{}, "--package-filter=<regex>, can be specified multiple times.")
}

func main() {
	if err := rootCmd.Execute(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	for _, s := range Options.PackageFilters {
		Options.PackageFiltersRegexes = append(Options.PackageFiltersRegexes, regexp.MustCompile(s))
	}

	Execute()
}
