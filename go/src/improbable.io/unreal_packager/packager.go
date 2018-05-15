// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/spf13/cobra"
)

const (
	buildTargetFlag        = "target"
	buildPlatformFlag      = "platform"
	buildConfigurationFlag = "configuration"
	projectFlag            = "project"
	engineDirFlag          = "engineDir"
	homeEnvVar             = "UNREAL_HOME"
)

var (
	longText = `NOTE: This command is currently only supported on the Windows platform.`

	buildScriptLocation = map[string]string{
		"windows": filepath.Join("Build", "BatchFiles", "Build.bat"),
	}

	cleanScriptLocation = map[string]string{
		"windows": filepath.Join("Build", "BatchFiles", "Clean.bat"),
	}

	uatScriptLocation = map[string]string{
		"windows": filepath.Join("Build", "BatchFiles", "RunUAT.bat"),
	}
)

const runServerScript = `#!/bin/bash
NEW_USER=unrealworker
WORKER_ID=$1
WORKER_NAME=$2
shift 2

LD_LIBRARY_PATH="$$LIBRARY_PATHS$$${LD_LIBRARY_PATH}"
export LD_LIBRARY_PATH

# "2>/dev/null" silences errors by redirecting stderr to the null device. This is done to prevent errors when a machine attempts to add the same user more than once.
useradd $NEW_USER -m -d /improbable/logs/UnrealWorker/Logs 2>/dev/null
chown -R $NEW_USER:$NEW_USER $(pwd)
chmod -R o+rw /improbable/logs
SCRIPT="$(pwd)/${WORKER_NAME}Server.sh"
chmod +x $SCRIPT
gosu $NEW_USER "${SCRIPT}" "$@" 2> >(grep -v xdg-user-dir >&2)`

const runEditorScript = `
setlocal ENABLEDELAYEDEXPANSION
%%UNREAL_HOME%%\Engine\Binaries\Win64\UE4Editor.exe "%v" %%*
exit /b !ERRORLEVEL!
`

func InitCmd(parentCmd *cobra.Command) {
	buildCmd := &cobra.Command{
		Use:   "build",
		Short: "Runs Unreal's Build script for an Unreal project in the current directory.",
		Long:  longText,
		RunE: func(cmd *cobra.Command, args []string) error {
			return runBuildScriptInCurrentDirectory(cmd, buildScriptLocation[runtime.GOOS], args)
		},
	}

	packageCmd := &cobra.Command{
		Use:   "package",
		Short: "Runs Unreal's package scripts on a specific project and packages the Unreal packages in the SpatialOS format.",
		Long:  longText,
		RunE: func(cmd *cobra.Command, args []string) error {
			projectFile, err := getProjectFile(cmd)
			if err != nil {
				return err
			}

			platform, _ := cmd.Flags().GetString(buildPlatformFlag)
			targetType, _ := cmd.Flags().GetString(buildTargetFlag)

			applicationRoot, err := getProjectRoot()
			if err != nil {
				return err
			}

			stagingDir := filepath.Join(applicationRoot, "build", "unreal", filepath.Base(projectFile))

			if targetType == "Editor" {
				platformStagingPath := filepath.Join(stagingDir, platform+targetType)
				targetFile := filepath.Join(platformStagingPath, "StartServer.bat")

				text := fmt.Sprintf(runEditorScript, projectFile)
				if err := writeTextFile(targetFile, text); err != nil {
					return err
				}

				outputPath := filepath.Join(applicationRoot, "build", "assembly", "worker", fmt.Sprintf("Unreal%v@Windows", targetType))

				return Zip(outputPath, platformStagingPath, "", useCompression)
			}

			if err := runUnrealCookingAndPackaging(cmd, stagingDir, args); err != nil {
				return err
			}

			if platform == "Linux" && targetType == "Server" {
				platformStagingPath := filepath.Join(stagingDir, "LinuxServer")
				targetFile := filepath.Join(platformStagingPath, "StartServer.sh")

				libraryPath, err := findLinuxLibraryPath(platformStagingPath)
				if err != nil {
					return err
				}

				expandedScript := strings.Replace(runServerScript, "$$LIBRARY_PATHS$$", libraryPath, -1)
				expandedScript = strings.Replace(expandedScript, "\r\n", "\n", -1)

				if err := writeTextFile(targetFile, expandedScript); err != nil {
					return err
				}
				if err := os.Chmod(targetFile, 0777); err != nil {
					return err
				}
			}
			return archiveWorkerPackages(applicationRoot, stagingDir, targetType, platform)
		},
	}

	cleanCmd := &cobra.Command{
		Use:   "clean",
		Short: "Runs Unreal's Clean script for an Unreal project in the current directory.",
		Long:  longText,
		RunE: func(cmd *cobra.Command, args []string) error {
			return runBuildScriptInCurrentDirectory(cmd, cleanScriptLocation[runtime.GOOS], args)
		},
	}

	addCommonFlags(buildCmd, cleanCmd, packageCmd)
	parentCmd.AddCommand(buildCmd, packageCmd, cleanCmd)
}

func runBuildScriptInCurrentDirectory(cmd *cobra.Command, scriptName string, args []string) error {
	if err := checkPlatformIsSupported(); err != nil {
		return err
	}

	binaryLocation, err := findBinary(cmd, scriptName)
	if err != nil {
		return err
	}

	buildTargetSuffix, _ := cmd.Flags().GetString(buildTargetFlag)
	buildPlatform, _ := cmd.Flags().GetString(buildPlatformFlag)
	buildConfiguration, _ := cmd.Flags().GetString(buildConfigurationFlag)

	projectFile, err := getProjectFile(cmd)
	if err != nil {
		return err
	}

	projectName := strings.Replace(filepath.Base(projectFile), ".uproject", "", 1)
	buildTarget := projectName + buildTargetSuffix

	allArgs := append([]string{buildTarget, buildPlatform, buildConfiguration, projectFile}, args...)

	unrealCommand := exec.Command(binaryLocation, allArgs...)

	unrealCommand.Stdout = os.Stdout
	unrealCommand.Stderr = os.Stderr

	return unrealCommand.Run()
}

func runUnrealCookingAndPackaging(cmd *cobra.Command, stagingDir string, additionalArgs []string) error {
	projectFile, err := getProjectFile(cmd)
	if err != nil {
		return err
	}

	platform, _ := cmd.Flags().GetString(buildPlatformFlag)
	targetType, _ := cmd.Flags().GetString(buildTargetFlag)
	configuration, _ := cmd.Flags().GetString(buildConfigurationFlag)

	inputArguments := append([]string{
		"BuildCookRun",
		"-project=" + projectFile,
		"-noP4",
		"-clientconfig=" + configuration,
		"-serverconfig=" + configuration,
		"-utf8output",
		"-cook",
		"-stage",
		"-package",
		"-unversioned",
		"-compressed",
		"-stagingdirectory=" + stagingDir,
		"-stdout",
		"-FORCELOGFLUSH",
		"-CrashForUAT",
		"-unattended",
		"-fileopenlog",
		"-SkipCookingEditorContent",
		"-nocompile",
		"-nocompileeditor",
	})

	if targetType == "Server" {
		inputArguments = append(inputArguments, []string{
			"-server",
			"-serverplatform=" + platform,
			"-noclient",
		}...)
	} else {
		inputArguments = append(inputArguments, []string{
			"-platform=" + platform,
			"-targetplatform=" + platform,
		}...)
	}

	inputArguments = append(inputArguments, additionalArgs...)

	command, err := findBinary(cmd, uatScriptLocation[runtime.GOOS])
	if err != nil {
		return err
	}

	unrealCommand := exec.Command(command, inputArguments...)

    unrealCommand.Stdout = os.Stdout
	unrealCommand.Stderr = os.Stderr

	return unrealCommand.Run()
}

func archiveWorkerPackages(applicationRoot string, stagingDir string, targetType string, platform string) error {
	platformLabel := platform

	// Windows has Win32/Win64 platform for the same label
	if strings.HasPrefix(strings.ToLower(platform), "win") {
		platformLabel = "Windows"
	}

	sourceFolder := platformLabel + "NoEditor"

	// PS4 has no Editor/NoEditor options for staging
	if platform == "PS4" {
		sourceFolder = platformLabel
	}
	outputFileName := "UnrealClient@" + platformLabel

	if targetType == "Server" {
		outputFileName = "UnrealWorker@" + platformLabel
		sourceFolder = platformLabel + "Server"
	}

	basePath := filepath.Join(stagingDir, sourceFolder)
	outputPath := filepath.Join(applicationRoot, "build", "assembly", "worker", outputFileName)

	return Zip(outputPath, basePath, "", useCompression)
}

// Write the contents of a file as UTF8, creating a directory structure as appropriate and overwriting any existing file
func writeTextFile(fileName, fileContent string) error {
	err := os.MkdirAll(filepath.Dir(fileName), os.ModeDir|0700)
	if err != nil {
		return err
	}

	writer, err := os.Create(fileName)
	if err != nil {
		return err
	}
	defer writer.Close()

	_, err = writer.WriteString(fileContent)
	return err
}

func getProjectRoot() (string, error) {
	projectRoot := ""

	cwd, err := os.Getwd()
	if err != nil {
		return "", err
	}

	path := cwd

	for {
		fileName := filepath.Join(path, "spatialos.json")

		stat, err := os.Stat(fileName)
		if err == nil && !stat.IsDir() {
			projectRoot = path
			break
		}

		path, err = filepath.Abs(filepath.Join(path, ".."))
		if err != nil {
			break
		}
	}

	if len(projectRoot) == 0 {
		cwd, _ := os.Getwd()

		return "", fmt.Errorf("could not find spatialos.json in any parent directory of '%v'", cwd)
	}

	return projectRoot, err
}
