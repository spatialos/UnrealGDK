Set-StrictMode -Version Latest

param(
  [string] $gdk_home = (get-item "$($PSScriptRoot)").parent.FullName, ## The root of the UnrealGDK repo
  [string] $gcs_publish_bucket = "io-internal-infra-unreal-artifacts-production/UnrealEngine",
  [string] $msbuild_exe = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2017\BuildTools\MSBuild\15.0\Bin\MSBuild.exe",
  [string] $target_platform = "Win64",
  [string] $build_home = (get-item "$($PSScriptRoot)").parent.parent.FullName, ## The root of the entire build
  [string] $unreal_path = "$build_home\UnrealEngine",
  [string] $testing_project_name = "StarterContent" ## For now, has to be inside the Engine's Samples folder
)

. "$PSScriptRoot\common.ps1"

# Download Unreal Engine
Start-Event "get-unreal-engine" "command"
&$PSScriptRoot"\get-engine.ps1"
Finish-Event "get-unreal-engine" "command"

# Run the required setup steps
Start-Event "setup-gdk" "command"
&$PSScriptRoot"\setup-gdk.ps1"
Finish-Event "setup-gdk" "command"

# Build the GDK plugin
Start-Event "build-gdk" "command"
&$PSScriptRoot"\build-gdk.ps1" -target_platform $($target_platform)
Finish-Event "build-gdk" "command"

Start-Event "setup-tests" "command"
&$PSScriptRoot"\setup-tests.ps1" -build_output_dir "$build_home\SpatialGDKBuild" -project_path "$unreal_path\Samples\$testing_project_name" -unreal_path $unreal_path
Finish-Event "setup-tests" "command"

Start-Event "test-gdk" "command"
&$PSScriptRoot"\run-tests.ps1" -ue_path "$unreal_path\Engine\Binaries\$target_platform\UE4Editor.exe" -uproject_path "$unreal_path\Samples\$testing_project_name\$testing_project_name.uproject" -output_dir "$PSScriptRoot\TestResults" -log_file_name "tests.log"
Finish-Event "test-gdk" "command"

# steps:
# get engine
# set up gdk
#   symlink gdk into engine plugins folder for expected folder structure
#   run setup.bat
# build plugin 
#   Using UAT, into a folder outside the engine
#   copy built plugin into SpatialGDK
# Run tests