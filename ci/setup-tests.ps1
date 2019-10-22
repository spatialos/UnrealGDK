# Expects gdk_home
param(
    [string] $build_output_dir,
    [string] $project_path,
    [string] $unreal_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\UnrealEngine" ## This should ultimately resolve to "C:\b\<number>\UnrealEngine".
)

# Copy the built files back into the SpatialGDK folder, to have a complete plugin
# The trailing \ on the destination path is important!
Copy-Item -Path "$build_output_dir\*" -Destination "$gdk_home\SpatialGDK\" -Recurse -Container -ErrorAction SilentlyContinue

# The Plugin does not get recognised as an Engine plugin, because we are using a pre-built version of the engine
# copying the plugin into the project's folder bypasses the issue
New-Item -Path "$project_path" -Name "Plugins" -ItemType "directory" -ErrorAction SilentlyContinue
New-Item -ItemType Junction -Name "UnrealGDK" -Path "$project_path\Plugins" -Target "$gdk_home"

# Create the TestResults directory if it does not exist, for storing results
New-Item -Path "$PSScriptRoot" -Name "TestResults" -ItemType "directory" -ErrorAction SilentlyContinue

# Disable tutorials, otherwise the closing of the window will crash the editor due to some graphic context reason
Add-Content -Path "$unreal_path\Engine\Config\BaseEditorSettings.ini" -Value "`r`n[/Script/IntroTutorials.TutorialStateSettings]`r`nTutorialsProgress=(Tutorial=/Engine/Tutorial/Basics/LevelEditorAttract.LevelEditorAttract_C,CurrentStage=0,bUserDismissed=True)`r`n"
Add-Content -Path "$project_path\Game\Config\DefaultGame.ini" -Value "`r`n[/Script/EngineSettings.GeneralProjectSettings]`r`nbSpatialNetworking=True`r`n"
