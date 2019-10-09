# Expects gdk_home
param(
    [string] $build_output_dir,
    [string] $project_path,
    [string] $output_dir,
    [string] $unreal_path
)

# Copy the built files back into the SpatialGDK folder, to have a complete plugin
# The trailing \ on the destination path is important!
Copy-Item -Path "$build_output_dir\*" -Destination "$gdk_home\SpatialGDK\" -Recurse -Container -ErrorAction SilentlyContinue

# The Plugin does not get recognised as an Engine plugin, because we are using a pre-built version of the engine
# copying the plugin into the project's folder bypasses the issue
New-Item -Path "$project_path" -Name "Plugins" -ItemType "directory" -ErrorAction SilentlyContinue
New-Item -ItemType Junction -Path "$project_path\Plugins\UnrealGDK" -Target "$gdk_home" -ErrorAction SilentlyContinue

# Create the TestResults directory if it does not exist, for storing results
New-Item -Path "$PSScriptRoot" -Name "TestResults" -ItemType "directory" -ErrorAction SilentlyContinue

# Disable tutorials, otherwise the closing of the window will crash the editor due to some graphic context reason
Add-Content -Path "$unreal_path\Engine\Config\BaseEditorSettings.ini" -Value "`r`n[/Script/IntroTutorials.TutorialStateSettings]`r`nbDismissedAllTutorials=True`r`nTutorialsProgress=(Tutorial=/Engine/Tutorial/Basics/LevelEditorOverview.LevelEditorOverview_C,CurrentStage=1,bUserDismissed=True)`r`nTutorialsProgress=(Tutorial=/Engine/Tutorial/Basics/LevelEditorAttract.LevelEditorAttract_C,CurrentStage=0,bUserDismissed=True)`r`n"

    # copy the built plugin into the engine
# Copy-Item $gdk_build_path "$($unreal_path)\Engine\Plugins\SpatialGDK" -Recurse
# Write-Log "plugins contents: $(Get-ChildItem $($unreal_path)\Engine\Plugins)"
# Write-Log "spatialgdk contents: $(Get-ChildItem $($unreal_path)\Engine\Plugins\SpatialGDK)"
