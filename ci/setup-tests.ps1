# Expects gdk_home
param(
    [string] $build_output_dir,
    [string] $project_path,
    [string] $unreal_path
)

# Copy the built files back into the SpatialGDK folder, to have a complete plugin
# The trailing \ on the destination path is important!
Copy-Item -Path "$build_output_dir\*" -Destination "$gdk_home\SpatialGDK\" -Recurse -Container -ErrorAction SilentlyContinue

# For some reason, the gdk plugin doesn't get loaded by the engine as an engine plugin, so also symlink it into the project we run the tests in
# TODO debug why it doesn't get loaded as an engine plugin
New-Item -Path "$project_path" -Name "Plugins" -ItemType "directory"
cmd /c mklink /J "$project_path\Plugins\UnrealGDK" "$gdk_home"

# Pretend we're doing an internal build by creating the file UnrealEngine\Engine\Build\NotForLicensees\EpicInternal.txt
# This is a file checked by unreal to determine whether it's running an internal build
# This disables showing the editor startup tutorial, which we need to do since running it crashes the editor
# The tutorial logic is in UnrealEngine/Engine/Source/Editor/IntorTutorials/Private/IntorTutorials.cpp, function MaybeopenWelcomeTutorial
# The check for this file is in unrealEngine/Engine/Source/Runtime/Core/Private/Misc/EngineBuildSettings.cpp, function IsInternalBuild
New-Item -Path "$unreal_path\Engine\Build" -Name "NotForLicensees" -ItemType "directory"
New-Item -Path "$unreal_path\Engine\Build\NotForLicensees" -Name "EpicInternal.txt" -ItemType "file"

    # copy the built plugin into the engine
# Copy-Item $gdk_build_path "$($unreal_path)\Engine\Plugins\SpatialGDK" -Recurse
# Write-Log "plugins contents: $(Get-ChildItem $($unreal_path)\Engine\Plugins)"
# Write-Log "spatialgdk contents: $(Get-ChildItem $($unreal_path)\Engine\Plugins\SpatialGDK)"
