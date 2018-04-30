// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

package main

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/spf13/cobra"
)

func getProjectFile(cmd *cobra.Command) (string, error) {
	projectFile, _ := cmd.Flags().GetString(projectFlag)
	if projectFile != "" {
		return projectFile, nil
	}

	fullPath, err := filepath.Abs(filepath.Join(".", "Game"))
	if err != nil {
		return "", err
	}

	files, err := filepath.Glob(filepath.Join(fullPath, "*.uproject"))
	if err != nil {
		return "", err
	}

	if len(files) == 0 {
		return "", fmt.Errorf("Could not find any uproject files in %v", fullPath)
	}

	return files[0], nil
}

func findBinary(cmd *cobra.Command, relativeBinaryPath string) (string, error) {
	enginePath, _ := cmd.Flags().GetString(engineDirFlag)

	stat, err := os.Stat(enginePath)
	if err != nil || !stat.IsDir() {
		return "", fmt.Errorf("Failed to find UnrealEngine directory. Please set the %v environment variable or check your --%v override path.", homeEnvVar, engineDirFlag)
	}

	location := filepath.Join(enginePath, relativeBinaryPath)
	location, err = filepath.Abs(location)
	if err != nil {
		return "", err
	}

	stat, err = os.Stat(location)
	if err != nil || stat.IsDir() {
		return "", fmt.Errorf("Could not find '%v' in '%v'", relativeBinaryPath, location)
	}
	return location, nil

}

func checkPlatformIsSupported() error {
	if runtime.GOOS != "windows" {
		return errors.New("Only the Windows platform is supported for this command.")
	}
	return nil
}

// findLinuxLibraryPath recursively searches for .so files and returns a string suitable for using as
// a value for the LD_LIBRARY_PATH environment variable.
func findLinuxLibraryPath(platformStagingPath string) (string, error) {
	dynamicPaths := []string{}

	err := filepath.Walk(platformStagingPath, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		if filepath.Ext(path) == ".so" {
			dynamicPaths = append(dynamicPaths, path)
		}
		return nil
	})

	if err != nil {
		return "", err
	}

	libraryPath := ""
	uniquePaths := map[string]struct{}{}

	for _, path := range dynamicPaths {
		relativePath, err := filepath.Rel(platformStagingPath, path)

		if err != nil {
			return "", err
		}

		relativePath = filepath.Dir(relativePath)
		relativePath = strings.Replace(relativePath, "\\", "/", -1)
		uniquePaths[relativePath] = struct{}{}
	}

	for key := range uniquePaths {
		libraryPath = key + ":" + libraryPath
	}

	return libraryPath, nil
}
