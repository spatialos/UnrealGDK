param(
  [string] $gdk_home = (Get-Item "$($PSScriptRoot)").parent.FullName, ## The root of the UnrealGDK repo
  [string] $gcs_publish_bucket = "io-internal-infra-unreal-artifacts-production/UnrealEngine",
  [string] $msbuild_exe = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2017\BuildTools\MSBuild\15.0\Bin\MSBuild.exe",
  [string] $target_platform = "Win64",
  [string] $build_home = (Get-Item "$($PSScriptRoot)").parent.parent.FullName, ## The root of the entire build
  [string] $unreal_path = "$build_home\UnrealEngine",
  [string] $testing_project_name = "StarterContent" ## For now, has to be inside the Engine's Samples folder
)

. "$PSScriptRoot\common.ps1"

# Download Unreal Engine
Start-Event "get-unreal-engine" "command"
&$PSScriptRoot"\get-engine.ps1" -unreal_path $unreal_path
Finish-Event "get-unreal-engine" "command"

# Run the required setup steps
Start-Event "setup-gdk" "command"
&$PSScriptRoot"\setup-gdk.ps1" -unreal_path $unreal_path
Finish-Event "setup-gdk" "command"

# Build the GDK plugin
Start-Event "build-gdk" "command"
&$PSScriptRoot"\build-gdk.ps1" -target_platform $($target_platform) -build_output_dir "$build_home\SpatialGDKBuild" -unreal_path $unreal_path
Finish-Event "build-gdk" "command"

# Only run tests on Windows, as we do not have a linux agent - should not matter
if ($target_platform -eq "Win64") {
  Start-Event "setup-tests" "command"
  &$PSScriptRoot"\setup-tests.ps1" -build_output_dir "$build_home\SpatialGDKBuild" -project_path "$unreal_path\Samples\$testing_project_name" -unreal_path $unreal_path
  Finish-Event "setup-tests" "command"

  Start-Event "test-gdk" "command"
  &$PSScriptRoot"\run-tests.ps1" -unreal_editor_path "$unreal_path\Engine\Binaries\$target_platform\UE4Editor.exe" -uproject_path "$unreal_path\Samples\$testing_project_name\$testing_project_name.uproject" -output_dir "$PSScriptRoot\TestResults" -log_file_name "tests.log"
  Finish-Event "test-gdk" "command"

  Start-Event "report-tests" "command"
  &$PSScriptRoot"\report-tests.ps1" -test_result_dir "$PSScriptRoot\TestResults"
  Finish-Event "report-tests" "command"
}
