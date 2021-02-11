param(
    [string] $gdk_home = (Get-Item "$($PSScriptRoot)").parent.FullName, ## The root of the UnrealGDK repo
    [string] $gcs_publish_bucket = "io-internal-infra-unreal-artifacts-production/UnrealEngine",
    [string] $msbuild_exe = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe",
    [string] $build_home = (Get-Item "$($PSScriptRoot)").parent.parent.FullName, ## The root of the entire build. Should ultimately resolve to "C:\b\<number>\".
    [string] $unreal_engine_symlink_dir = "$build_home\UnrealEngine",
    [string] $gyms_version_path = "$gdk_home\UnrealGDKTestGymsVersion.txt"
)

class TestProjectTarget {
    [ValidateNotNullOrEmpty()][string]$test_repo_url
    [ValidateNotNullOrEmpty()][string]$test_repo_branch
    [ValidateNotNullOrEmpty()][string]$test_repo_relative_uproject_path
    [ValidateNotNullOrEmpty()][string]$test_project_name
    
    TestProjectTarget([string]$test_repo_url, [string]$gdk_branch, [string]$test_repo_relative_uproject_path, [string]$test_project_name, [string]$test_gyms_version_path, [string]$test_env_override) {
        $this.test_repo_url = $test_repo_url
        $this.test_repo_relative_uproject_path = $test_repo_relative_uproject_path
        $this.test_project_name = $test_project_name
        
        # Resolve the branch to run against. The order of priority is:
        # envvar > same-name branch as the branch we are currently on > UnrealGDKTestGymVersion.txt > "master".
        $testing_repo_heads = git ls-remote --heads $test_repo_url $gdk_branch
        $test_gym_version = if (Test-Path -Path $test_gyms_version_path) { [System.IO.File]::ReadAllText($test_gyms_version_path) } else { [string]::Empty }
        if (Test-Path $test_env_override) {
            $this.test_repo_branch = (Get-Item $test_env_override).value
        }
        elseif ($testing_repo_heads -Match [Regex]::Escape("refs/heads/$gdk_branch")) {
            $this.test_repo_branch = $gdk_branch
        }
        elseif ($test_gym_version -ne [string]::Empty) {
            $this.test_repo_branch = $test_gym_version
        }
        else {
            $this.test_repo_branch = "master"
        }
    }
}

class TestSuite {
    [ValidateNotNullOrEmpty()][TestProjectTarget]$test_project_target
    [ValidateNotNullOrEmpty()][string]$test_repo_map
    [ValidateNotNullOrEmpty()][string]$test_results_dir
    [ValidateNotNullOrEmpty()][string]$tests_path
    [ValidateNotNull()]       [string]$additional_gdk_options
    [bool]                            $run_with_spatial
    [ValidateNotNull()]       [string]$additional_cmd_line_args

    TestSuite([TestProjectTarget] $test_project_target, [string] $test_repo_map,
        [string] $test_results_dir, [string] $tests_path, [string] $additional_gdk_options,
        [bool] $run_with_spatial, [string] $additional_cmd_line_args) {
        $this.test_project_target = $test_project_target
        $this.test_repo_map = $test_repo_map
        $this.test_results_dir = $test_results_dir
        $this.tests_path = $tests_path
        $this.additional_gdk_options = $additional_gdk_options
        $this.run_with_spatial = $run_with_spatial
        $this.additional_cmd_line_args = $additional_cmd_line_args
    }
}

[string] $user_gdk_settings = "$env:GDK_SETTINGS"
[string] $user_cmd_line_args = "$env:TEST_ARGS"
[string] $gdk_branch = "$env:BUILDKITE_BRANCH"
[string] $test_map_path = "/Game/Intermediate/Maps/"

[TestProjectTarget] $gdk_test_project = [TestProjectTarget]::new("git@github.com:spatialos/UnrealGDKTestGyms.git", $gdk_branch, "Game\GDKTestGyms.uproject", "GDKTestGyms", $gyms_version_path, "env:TEST_REPO_BRANCH")
[TestProjectTarget] $native_test_project = [TestProjectTarget]::new("git@github.com:improbable/UnrealGDKEngineNetTest.git", $gdk_branch, "Game\EngineNetTest.uproject", "NativeNetworkTestProject", $gyms_version_path, "env:NATIVE_TEST_REPO_BRANCH")

$tests = @()

if ((Test-Path env:TEST_CONFIG) -And ($env:TEST_CONFIG -eq "Native")) {
    # We run spatial tests against Vanilla UE4
    $tests += [TestSuite]::new($gdk_test_project, "SpatialNetworkingMap", "VanillaTestResults", "${test_map_path}CI_Premerge/", "$user_gdk_settings", $False, "$user_cmd_line_args")

    if ($env:SLOW_NETWORKING_TESTS -like "true") {
        $tests[0].tests_path += "+${test_map_path}CI_Nightly/"
        $tests[0].test_results_dir = "Slow" + $tests[0].test_results_dir

        # And if slow, we run NetTest functional maps against Vanilla UE4 as well
        $tests += [TestSuite]::new($native_test_project, "NetworkingMap", "NativeNetTestResults", "/Game/NetworkingMap", "$user_gdk_settings", $False, "$user_cmd_line_args")
    }
}
else {
    # We run all tests and networked functional maps
    $tests += [TestSuite]::new($gdk_test_project, "SpatialNetworkingMap", "TestResults", "SpatialGDK.+${test_map_path}CI_Premerge/+${test_map_path}CI_Premerge_Spatial_Only/", "$user_gdk_settings", $True, "$user_cmd_line_args")

    if ($env:SLOW_NETWORKING_TESTS -like "true") {
        # And if slow, we run GDK slow tests
        $tests[0].tests_path += "+SpatialGDKSlow.+${test_map_path}CI_Nightly/+${test_map_path}CI_Nightly_Spatial_Only/"
        $tests[0].test_results_dir = "Slow" + $tests[0].test_results_dir

        # And NetTests functional maps against GDK as well
        $tests += [TestSuite]::new($native_test_project, "NetworkingMap", "GDKNetTestResults", "/Game/NetworkingMap", "$user_gdk_settings", $True, "$user_cmd_line_args")
    }
}

. "$PSScriptRoot\common.ps1"

# Guard against other runs not cleaning up after themselves
Foreach ($test in $tests) {
    $test_project_name = $test.test_project_target.test_project_name
    & $PSScriptRoot"\cleanup.ps1" `
        -project_name "$test_project_name"
}

# Download Unreal Engine
Start-Event "get-unreal-engine" "command"
& $PSScriptRoot"\get-engine.ps1" -unreal_path "$unreal_engine_symlink_dir"
Finish-Event "get-unreal-engine" "command"

# Run the required setup steps
Start-Event "setup-gdk" "command"
& $PSScriptRoot"\setup-gdk.ps1" -gdk_path "$gdk_in_engine" -msbuild_path "$msbuild_exe"
Finish-Event "setup-gdk" "command"

class CachedProject {
    [ValidateNotNullOrEmpty()][string]$test_repo_url
    [ValidateNotNullOrEmpty()][string]$test_repo_branch

    CachedProject([string] $test_repo_url, [string] $test_repo_branch) {
        $this.test_repo_url = $test_repo_url
        $this.test_repo_branch = $test_repo_branch
    }
}

$projects_cached = @()

Foreach ($test in $tests) {
    $test_repo_url = $test.test_project_target.test_repo_url
    $test_repo_branch = $test.test_project_target.test_repo_branch
    $test_repo_relative_uproject_path = $test.test_project_target.test_repo_relative_uproject_path
    $test_project_name = $test.test_project_target.test_project_name
    $test_repo_map = $test.test_repo_map
    $test_results_dir = $test.test_results_dir
    $tests_path = $test.tests_path
    $additional_gdk_options = $test.additional_gdk_options
    $run_with_spatial = $test.run_with_spatial
    $additional_cmd_line_args = $test.additional_cmd_line_args

    $project_is_cached = $False
    Foreach ($cached_project in $projects_cached) {
        if (($test_repo_url -eq $cached_project.test_repo_url) -and ($test_repo_branch -eq $cached_project.test_repo_branch)) {
            $project_is_cached = $True
        }
    }

    if (-Not $project_is_cached) {
        # Build the testing project
        Start-Event "build-project" "command"
        & $PSScriptRoot"\build-project.ps1" `
            -unreal_path "$unreal_engine_symlink_dir" `
            -test_repo_branch "$test_repo_branch" `
            -test_repo_url "$test_repo_url" `
            -test_repo_uproject_path "$build_home\$test_project_name\$test_repo_relative_uproject_path" `
            -test_repo_path "$build_home\$test_project_name" `
            -msbuild_exe "$msbuild_exe" `
            -gdk_home "$gdk_home" `
            -build_platform "$env:BUILD_PLATFORM" `
            -build_state "$env:BUILD_STATE" `
            -build_target "$env:BUILD_TARGET"

        $projects_cached += [CachedProject]::new($test_repo_url, $test_repo_branch)
        Finish-Event "build-project" "command"
    }

    # Only run tests on Windows, as we do not have a linux agent - should not matter
    if ($env:BUILD_PLATFORM -eq "Win64" -And $env:BUILD_TARGET -eq "Editor" -And $env:BUILD_STATE -eq "Development") {
        Start-Event "test-gdk" "command"
        $verify_commandlet_exit_codes = $False # For now, we will ignore the exit codes of the commandlets run, as older engine versions with TestGyms produce errors
        & $PSScriptRoot"\run-tests.ps1" `
            -unreal_editor_path "$unreal_engine_symlink_dir\Engine\Binaries\Win64\UE4Editor.exe" `
            -uproject_path "$build_home\$test_project_name\$test_repo_relative_uproject_path" `
            -test_repo_path "$build_home\$test_project_name" `
            -log_file_path "$PSScriptRoot\$test_project_name\$test_results_dir\tests.log" `
            -report_output_path "$test_project_name\$test_results_dir" `
            -test_repo_map "$test_repo_map" `
            -tests_path "$tests_path" `
            -additional_gdk_options "$additional_gdk_options" `
            -run_with_spatial $run_with_spatial `
            -additional_cmd_line_args "$additional_cmd_line_args" `
            -verify_commandlet_exit_codes $verify_commandlet_exit_codes
        Finish-Event "test-gdk" "command"

        Start-Event "report-tests" "command"
        & $PSScriptRoot"\report-tests.ps1" -test_result_dir "$PSScriptRoot\$test_project_name\$test_results_dir" -target_platform "$env:BUILD_PLATFORM"
        Finish-Event "report-tests" "command"
    }
}
