param(
  [string] $gdk_home = (Get-Item "$($PSScriptRoot)").parent.FullName, ## The root of the UnrealGDK repo
  [string] $gcs_publish_bucket = "io-internal-infra-unreal-artifacts-production/UnrealEngine",
  [string] $msbuild_exe = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2017\BuildTools\MSBuild\15.0\Bin\MSBuild.exe",
  [string] $target_platform = "Win64",
  [string] $build_home = (Get-Item "$($PSScriptRoot)").parent.parent.FullName, ## The root of the entire build. Should ultimately resolve to "C:\b\<number>\".
  [string] $unreal_path = "$build_home\UnrealEngine",
  [string] $test_repo_branch = "master",
  [string] $test_repo_url = "https://github.com/spatialos/UnrealGDKTestGyms.git",
  [string] $test_repo_relative_uproject_path = "Game\GDKTestGyms.uproject",
  [string] $test_repo_map = "EmptyGym",
  [string] $spatial_cli_version = "20191106.121025.bda19848a2"
)

# Allow overriding testing branch via environment variable
if (Test-Path env:TEST_REPO_BRANCH) {
  $test_repo_branch = $env:TEST_REPO_BRANCH
}

. "$PSScriptRoot\common.ps1"

Start-Event "cleanup-symlinks" "command"
&$PSScriptRoot"\cleanup.ps1" -unreal_path "$unreal_path"
Finish-Event "cleanup-symlinks" "command"

# Download Unreal Engine
Start-Event "get-unreal-engine" "command"
&$PSScriptRoot"\get-engine.ps1" -unreal_path "$unreal_path"
Finish-Event "get-unreal-engine" "command"

Start-Event "symlink-gdk" "command"
$gdk_in_engine = "$unreal_path\Engine\Plugins\UnrealGDK"
New-Item -ItemType Junction -Name "UnrealGDK" -Path "$unreal_path\Engine\Plugins" -Target "$gdk_home"
Finish-Event "symlink-gdk" "command"

# Run the required setup steps
Start-Event "setup-gdk" "command"
&$PSScriptRoot"\setup-gdk.ps1" -gdk_path "$gdk_in_engine" -msbuild_path "$msbuild_exe"
Finish-Event "setup-gdk" "command"

# Build the GDK plugin
&$PSScriptRoot"\build-gdk.ps1" -target_platform $($target_platform) -build_output_dir "$build_home\SpatialGDKBuild" -unreal_path $unreal_path

# Update spatial to compatible version
$proc = Start-Process spatial "update","$spatial_cli_version" -Wait -ErrorAction Stop -NoNewWindow -PassThru
if ($proc.ExitCode -ne 0) {
  THROW "Failed to update spatial CLI to version $spatial_cli_version"
}

# Build the testing project
Start-Event "build-project" "command"
&$PSScriptRoot"\build-project.ps1" `
    -build_output_dir "$build_home\SpatialGDKBuild" `
    -unreal_path "$unreal_path" `
    -test_repo_branch "$test_repo_branch" `
    -test_repo_url "$test_repo_url" `
    -test_repo_uproject_path "$build_home\TestProject\$test_repo_relative_uproject_path" `
    -test_repo_path "$build_home\TestProject" `
    -msbuild_exe "$msbuild_exe" `
    -gdk_home "$gdk_home" `
    -build_platform "$build_platform",
    -build_state "$env:BUILD_STATE",
    -build_target "$env:BUILD_TARGET"
Finish-Event "build-project" "command"

# Only run tests on Windows, as we do not have a linux agent - should not matter
if ($target_platform -eq "Win64" -And $env:BUILD_TARGET -eq "Editor") {
  Start-Event "setup-tests" "command"
  &$PSScriptRoot"\setup-tests.ps1" `
    -unreal_path "$unreal_path" `
    -test_repo_uproject_path "$build_home\TestProject\$test_repo_relative_uproject_path" `
    -test_repo_map "$test_repo_map" `
    -test_repo_path "$build_home\TestProject"
  Finish-Event "setup-tests" "command"

  Start-Event "test-gdk" "command"
  &$PSScriptRoot"\run-tests.ps1" `
      -unreal_editor_path "$unreal_path\Engine\Binaries\Win64\UE4Editor.exe" `
      -uproject_path "$build_home\TestProject\$test_repo_relative_uproject_path" `
      -output_dir "$PSScriptRoot\TestResults" `
      -log_file_path "$PSScriptRoot\TestResults\tests.log" `
      -test_repo_map "$test_repo_map"
  Finish-Event "test-gdk" "command"

  Start-Event "report-tests" "command"
  &$PSScriptRoot"\report-tests.ps1" -test_result_dir "$PSScriptRoot\TestResults"
  Finish-Event "report-tests" "command"
}
