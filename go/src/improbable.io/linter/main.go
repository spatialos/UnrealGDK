// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

package main

import (
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"
)

func main() {
	rootCmd := &cobra.Command{
		SilenceUsage:  true,
		SilenceErrors: true,
	}

	checkCmd := &cobra.Command{
		Use: "check <paths>",
		RunE: func(cmd *cobra.Command, args []string) error {
			paths, err := getFailedValidationPaths(args)
			if err != nil {
				return err
			}

			for _, path := range paths {
				fmt.Fprintf(os.Stderr, "%v - missing copyright header\n", path)
			}

			if len(paths) > 0 {
				return fmt.Errorf("%v files failed linting", len(paths))
			}
			return nil
		},
	}

	fixCmd := &cobra.Command{
		Use: "fix <paths>",
		RunE: func(cmd *cobra.Command, args []string) error {
			paths, err := getFailedValidationPaths(args)
			if err != nil {
				return err
			}

			for _, path := range paths {
				fileContent, err := ioutil.ReadFile(path)
				if err != nil {
					return err
				}

				stringContent := string(fileContent)
				stringContent = "// Copyright (c) Improbable Worlds Ltd, All Rights Reserved\r\n\r\n" + stringContent

				info, err := os.Stat(path)
				if err != nil {
					return err
				}

				err = ioutil.WriteFile(path, []byte(stringContent), info.Mode())
				if err != nil {
					return err
				}

				fmt.Fprintf(os.Stdout, "Fixed %v\n", path)
			}

			return nil
		},
	}

	rootCmd.AddCommand(checkCmd, fixCmd)

	if err := rootCmd.Execute(); err != nil {
		fmt.Fprintf(os.Stderr, "Error: %v", err.Error())
		os.Exit(1)
	}

	fmt.Fprintln(os.Stdout, "All checks passed.")
}

func getFailedValidationPaths(inPaths []string) ([]string, error) {
	var outPaths []string

	for _, dir := range inPaths {
		dir, err := filepath.Abs(dir)
		if err != nil {
			return nil, err
		}

		walkErr := filepath.Walk(dir, func(path string, info os.FileInfo, err error) error {
			if err != nil {
				return err
			}

			if info.IsDir() {
				if isExcluded(path) {
					return filepath.SkipDir
				}
				return nil
			}

			ext := filepath.Ext(path)
			if ext != ".h" && ext != ".cpp" && !strings.HasSuffix(path, ".Build.cs") {
				return nil
			}

			if isExcluded(path) {
				return nil
			}

			if err := needsChanges(path); err != nil {
				outPaths = append(outPaths, path)
			}

			return nil
		})

		if walkErr != nil {
			return nil, err
		}
	}

	return outPaths, nil
}

func isExcluded(path string) bool {
	gitCmd := exec.Command("git", "check-ignore", path)
	err := gitCmd.Run()

	// `check-ignore` returns 0 if the file is ignored, and 1 if it's tracked.
	return err == nil
}

func needsChanges(fileName string) error {
	contentBytes, err := ioutil.ReadFile(fileName)
	if err != nil {
		return err
	}

	if !strings.HasPrefix(string(contentBytes), "// Copyright") {
		return fmt.Errorf("%v is missing the copyright header", fileName)
	}
	return nil
}
