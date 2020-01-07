param(
  [string] $gdk_home = (Get-Item "$($PSScriptRoot)").parent.FullName, ## The root of the UnrealGDK repo
  [string] $gcs_publish_bucket = "io-internal-infra-unreal-artifacts-production/UnrealEngine",
  [string] $msbuild_exe = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2017\BuildTools\MSBuild\15.0\Bin\MSBuild.exe",
  [string] $build_home = (Get-Item "$($PSScriptRoot)").parent.parent.FullName, ## The root of the entire build. Should ultimately resolve to "C:\b\<number>\".
  [string] $unreal_path = "$build_home\UnrealEngine"
)

$tests = @(
  ("https://github.com/spatialos/UnrealGDKTestGyms.git", "master", "Game\GDKTestGyms.uproject", "EmptyGym", "TestProject"),
  ("https://github.com/improbable/UnrealGDKEngineNetTest.git", "master", "Game\EngineNetTest.uproject", "NetworkingMap", "NetworkTestProject")
)

# Allow overriding testing branch via environment variable
if (Test-Path env:TEST_REPO_BRANCH) {
  $test_repo_branch = $env:TEST_REPO_BRANCH
}

. "$PSScriptRoot\common.ps1"

# Guard against other runs not cleaning up after themselves
& $PSScriptRoot"\cleanup.ps1"

# Download Unreal Engine
Start-Event "get-unreal-engine" "command"
& $PSScriptRoot"\get-engine.ps1" -unreal_path "$unreal_path"
Finish-Event "get-unreal-engine" "command"

# Run the required setup steps
Start-Event "setup-gdk" "command"
& $PSScriptRoot"\setup-gdk.ps1" -gdk_path "$gdk_in_engine" -msbuild_path "$msbuild_exe"
Finish-Event "setup-gdk" "command"

foreach ($test in $tests) {
  
  $test_repo_url = $test[0]
  $test_repo_branch = $test[1]
  $test_repo_relative_uproject_path = $test[2]
  $test_repo_map = $test[3]
  $test_project_root = $test[4]

  # Build the testing project
  Start-Event "build-project" "command"
  & $PSScriptRoot"\build-project.ps1" `
      -unreal_path "$unreal_path" `
      -test_repo_branch "$test_repo_branch" `
      -test_repo_url "$test_repo_url" `
      -test_repo_uproject_path "$build_home\$test_project_root\$test_repo_relative_uproject_path" `
      -test_repo_path "$build_home\$test_project_root" `
      -msbuild_exe "$msbuild_exe" `
      -gdk_home "$gdk_home" `
      -build_platform "$env:BUILD_PLATFORM" `
      -build_state "$env:BUILD_STATE" `
      -build_target "$env:BUILD_TARGET"
  Finish-Event "build-project" "command"

  # Only run tests on Windows, as we do not have a linux agent - should not matter
  if ($env:BUILD_PLATFORM -eq "Win64" -And $env:BUILD_TARGET -eq "Editor" -And $env:BUILD_STATE -eq "Development") {
    Start-Event "test-gdk" "command"
    & $PSScriptRoot"\run-tests.ps1" `
        -unreal_editor_path "$unreal_path\Engine\Binaries\Win64\UE4Editor.exe" `
        -uproject_path "$build_home\$test_project_root\$test_repo_relative_uproject_path" `
        -test_repo_path "$build_home\$test_project_root" `
        -log_file_path "$PSScriptRoot\$test_project_root\TestResults\tests.log" `
        -test_repo_map "$test_repo_map"
    Finish-Event "test-gdk" "command"

    Start-Event "report-tests" "command"
    & $PSScriptRoot"\report-tests.ps1" -test_result_dir "$PSScriptRoot\$test_project_root\TestResults" -target_platform "$env:BUILD_PLATFORM"
    Finish-Event "report-tests" "command"
  }
}

