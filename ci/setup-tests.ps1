# Expects gdk_home
param(
    [string] $build_output_dir,
    [string] $unreal_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\UnrealEngine", ## This should ultimately resolve to "C:\b\<number>\UnrealEngine".
    [string] $testing_repo_branch,
    [string] $testing_repo_url,
    [string] $uproject_path,
    [string] $msbuild_exe
)

# Copy the built files back into the SpatialGDK folder, to have a complete plugin
# The trailing \ on the destination path is important!
Copy-Item -Path "$build_output_dir\*" -Destination "$gdk_home\SpatialGDK\" -Recurse -Container -ErrorAction SilentlyContinue

# Clone and build the testing project
Start-Event "setup-project" "setup-tests"
$project_path = "$unreal_path\Samples\UnrealGDKCITestProject"
Try {
    if (Test-Path $project_path) {
        Write-Log "Removing existing project."
        Remove-Item $project_path -Recurse -Force
        if(-Not $?) {
            Throw "Failed to remove existing project at $($project_path)."
        }
    }
    Write-Log "Downloading the testing project from $($testing_repo_url)."
    Git clone -b $testing_repo_branch $testing_repo_url $unreal_path\Samples\UnrealGDKCITestProject
    if(-Not $?) {
        Throw "Failed to clone testing project from $($testing_repo_url)."
    }
    Write-Log "Generating project files."
    Start-Process $unreal_path\Engine\Binaries\DotNET\UnrealBuildTool.exe "-projectfiles","-project=`"$uproject_path`"","-game","-engine","-progress" -Wait -ErrorAction Stop -NoNewWindow
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to generate files for the testing project."
    }
    Write-Log "Building the testing project."
    Start-Process $msbuild_exe "$project_path\GDKShooter.sln","/property:Configuration=Development_Editor" -Wait -ErrorAction Stop -NoNewWindow
    #Start-Process $unreal_path\Engine\Build\BatchFiles\Build.bat "UnrealGDKCITestProject","Win64","Development_Editor","$uproject_path" -waitmutex
}
Catch {
    Throw $_
}
Finally {
    Finish-Event "setup-project" "setup-tests"
}

# The Plugin does not get recognised as an Engine plugin, because we are using a pre-built version of the engine
# copying the plugin into the project's folder bypasses the issue
New-Item -Path "$project_path" -Name "Plugins" -ItemType "directory" -ErrorAction SilentlyContinue
New-Item -ItemType Junction -Name "UnrealGDK" -Path "$project_path\Plugins" -Target "$gdk_home"

# Create the TestResults directory if it does not exist, for storing results
New-Item -Path "$PSScriptRoot" -Name "TestResults" -ItemType "directory" -ErrorAction SilentlyContinue

# Disable tutorials, otherwise the closing of the window will crash the editor due to some graphic context reason
Add-Content -Path "$unreal_path\Engine\Config\BaseEditorSettings.ini" -Value "`r`n[/Script/IntroTutorials.TutorialStateSettings]`r`nTutorialsProgress=(Tutorial=/Engine/Tutorial/Basics/LevelEditorAttract.LevelEditorAttract_C,CurrentStage=0,bUserDismissed=True)`r`n"
