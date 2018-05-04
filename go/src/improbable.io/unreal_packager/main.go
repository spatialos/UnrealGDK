// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

package main

import (
	"fmt"
	"os"
	"path/filepath"

	"github.com/spf13/cobra"
)

func main() {
	rootCmd := &cobra.Command{
		Use: filepath.Base(os.Args[0]),
	}
	rootCmd.SilenceUsage = true
	rootCmd.SilenceErrors = true

	InitCmd(rootCmd)

	err := rootCmd.Execute()
	if err != nil {
		fmt.Fprintf(os.Stderr, "%v\n", err)
		os.Exit(1)
	}
}

func addCommonFlags(commands ...*cobra.Command) {
	defaultEngineDir := os.Getenv(homeEnvVar)
	if defaultEngineDir == "" {
		defaultEngineDir = "."
	}

	defaultEngineDir = filepath.Join(defaultEngineDir, "Engine")

	for _, cmd := range commands {
		cmd.Flags().String(buildTargetFlag, "Editor", "The suffix of the target to build [Game,Server,Editor]")
		cmd.Flags().String(buildConfigurationFlag, "Development", "The configuration of the target to build [Debug,Development,Shipping]")
		cmd.Flags().String(buildPlatformFlag, "Win64", "The platform of the target to build [Win32,Win64,Linux]")
		cmd.Flags().String(projectFlag, "", "The Unreal project filename.")
		cmd.Flags().String(engineDirFlag, defaultEngineDir, "The path to the engine.")
	}
}
