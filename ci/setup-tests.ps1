# Expects gdk_home
param(
    [string] $build_output_dir,
    [string] $unreal_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\UnrealEngine", ## This should ultimately resolve to "C:\b\<number>\UnrealEngine".
    [string] $testing_repo_branch,
    [string] $testing_repo_url,
)

# Copy the built files back into the SpatialGDK folder, to have a complete plugin
# The trailing \ on the destination path is important!
Copy-Item -Path "$build_output_dir\*" -Destination "$gdk_home\SpatialGDK\" -Recurse -Container -ErrorAction SilentlyContinue

# Clone the testing project, or pull any changes if it has already been cloned
Start-Event "setup-project" "setup-tests"
$project_path = "$unreal_path\Samples\UnrealGDKCITestProject"
Try {
    if (Test-Path $project_path) {
        Write-Log "Project already exists, checking out $($testing_repo_branch) and pulling any changes"
        Git -C $project_path checkout -f $testing_repo_branch
        if(-Not $?) {
            Throw "Failed to checkout $($testing_repo_branch) branch of the testing project."
        }
        Git -C $project_path clean -df
        if(-Not $?) {
            Throw "Failed to clean the existing testing project."
        }
        Git -C $project_path pull $testing_repo_url $testing_repo_branch
        if(-Not $?) {
            Throw "Failed to pull changes to the existing testing project."
        }
    } else {
        Write-Log "Downloading the testing project from $($testing_repo_url)."
        Git clone -b $testing_repo_branch $testing_repo_url $unreal_path\Samples\UnrealGDKCITestProject
        if(-Not $?) {
            Throw "Failed to clone testing project from $($testing_repo_url)."
        }
    }
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
