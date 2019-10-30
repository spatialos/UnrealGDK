# Expects gdk_home
param(
    [string] $build_output_dir,
    [string] $unreal_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\UnrealEngine", ## This should ultimately resolve to "C:\b\<number>\UnrealEngine".
    [string] $test_repo_branch,
    [string] $test_repo_url,
    [string] $test_repo_map,
    [string] $test_repo_uproject_path,
    [string] $test_repo_path,
    [string] $msbuild_exe
)

# Copy the built files back into the SpatialGDK folder, to have a complete plugin
# The trailing \ on the destination path is important!
Copy-Item -Path "$build_output_dir\*" -Destination "$gdk_home\SpatialGDK\" -Recurse -Container -ErrorAction SilentlyContinue

# Update spatial to newest version
Start-Process spatial "update" -Wait -ErrorAction Stop -NoNewWindow

# Clean up testing project (symlinks could be invalid during initial cleanup - leaving the project as a result)
if (Test-Path $test_repo_path) {
    Write-Log "Removing existing project"
    Remove-Item $test_repo_path -Recurse -Force
    if (-Not $?) {
        Throw "Failed to remove existing project at $($test_repo_path)."
    }
}

# Clone and build the testing project
Write-Log "Downloading the testing project from $($test_repo_url)"
Git clone -b $test_repo_branch $test_repo_url $test_repo_path
if (-Not $?) {
    Throw "Failed to clone testing project from $($test_repo_url)."
}

# The Plugin does not get recognised as an Engine plugin, because we are using a pre-built version of the engine
# copying the plugin into the project's folder bypasses the issue
New-Item -ItemType Junction -Name "UnrealGDK" -Path "$test_repo_path\Game\Plugins" -Target "$gdk_home"

Write-Log "Generating project files"
Start-Process "$unreal_path\Engine\Binaries\DotNET\UnrealBuildTool.exe" "-projectfiles","-project=`"$test_repo_uproject_path`"","-game","-engine","-progress" -Wait -ErrorAction Stop -NoNewWindow
if (-Not $?) {
    throw "Failed to generate files for the testing project."
}
Write-Log "Building the testing project"
Start-Process "$msbuild_exe" "/nologo","$($test_repo_uproject_path.Replace(".uproject", ".sln"))","/p:Configuration=`"Development Editor`";Platform=`"Win64`"" -Wait -ErrorAction Stop -NoNewWindow
if (-Not $?) {
    throw "Failed to build testing project."
}

# Generate schema and snapshots
Write-Log "Generating snapshot and schema for testing project"
$commandlet_process = Start-Process "$unreal_path\Engine\Binaries\Win64\UE4Editor.exe" -Wait -PassThru -NoNewWindow -ArgumentList @(`
    "$test_repo_uproject_path", `
    "-run=GenerateSchemaAndSnapshots", `
    "-MapPaths=`"$test_repo_map`""
)
if (-Not $?) {
    Write-Host $commandlet_process.
    throw "Failed to generate schema and snapshots."
}

# Create the default snapshot
Copy-Item -Force `
    -Path "$test_repo_path\spatial\snapshots\$test_repo_map.snapshot" `
    -Destination "$test_repo_path\spatial\snapshots\default.snapshot"

# Create the TestResults directory if it does not exist, for storing results
New-Item -Path "$PSScriptRoot" -Name "TestResults" -ItemType "directory" -ErrorAction SilentlyContinue

# Disable tutorials, otherwise the closing of the window will crash the editor due to some graphic context reason
Add-Content -Path "$unreal_path\Engine\Config\BaseEditorSettings.ini" -Value "`r`n[/Script/IntroTutorials.TutorialStateSettings]`r`nTutorialsProgress=(Tutorial=/Engine/Tutorial/Basics/LevelEditorAttract.LevelEditorAttract_C,CurrentStage=0,bUserDismissed=True)`r`n"
