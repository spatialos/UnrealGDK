param(
    [string] $unreal_editor_path,
    [string] $uproject_path,
    [string] $test_repo_path,
    [string] $log_file_path,
    [string] $test_repo_map,
    [string] $report_output_path,
    [string] $tests_path = "SpatialGDK",
    [string] $additional_gdk_options,
    [bool]   $run_with_spatial
)

# This resolves a path to be absolute, without actually reading the filesystem.
# This means it works even when the indicated path does not exist, as opposed to the Resolve-Path cmdlet
function Force-ResolvePath {
    param (
        [string] $path
    )
    return $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($path)
}

if ($run_with_spatial) {
    # Generate schema and snapshots
    Write-Output "Generating snapshot and schema for testing project"
    Start-Process "$unreal_editor_path" -Wait -PassThru -NoNewWindow -ArgumentList @(`
        "$uproject_path", `
        "-NoShaderCompile", # Prevent shader compilation
        "-nopause", # Close the unreal log window automatically on exit
        "-nosplash", # No splash screen
        "-unattended", # Disable anything requiring user feedback
        "-nullRHI", # Hard to find documentation for, but seems to indicate that we want something akin to a headless (i.e. no UI / windowing) editor
        "-run=GenerateSchemaAndSnapshots", # Run the commandlet
        "-MapPaths=`"$test_repo_map`"" # Which maps to run the commandlet for
    )

    # Create the default snapshot
    Copy-Item -Force `
        -Path "$test_repo_path\spatial\snapshots\$test_repo_map.snapshot" `
        -Destination "$test_repo_path\spatial\snapshots\default.snapshot"
}

# Create the TestResults directory if it does not exist, for storing results
New-Item -Path "$PSScriptRoot" -Name "$report_output_path" -ItemType "directory" -ErrorAction SilentlyContinue
$output_dir = "$PSScriptRoot\$report_output_path"

# We want absolute paths since paths given to the unreal editor are interpreted as relative to the UE4Editor binary
# Absolute paths are more reliable
$ue_path_absolute = Force-ResolvePath $unreal_editor_path
$uproject_path_absolute = Force-ResolvePath $uproject_path
$output_dir_absolute = Force-ResolvePath $output_dir

Add-Content -Path "$uproject_path_absolute\Game\Config\DefaultSpatialGDKSettings.ini" -Value "`r`n`r`n$additional_gdk_options"

$cmd_args_list = @( `
    "`"$uproject_path_absolute`"", # We need some project to run tests in, but for unit tests the exact project shouldn't matter
    "`"$test_repo_map`"", # The map to run tests in
    "-ExecCmds=`"Automation RunTests $tests_path; Quit`"", # Run all tests. See https://docs.unrealengine.com/en-US/Programming/Automation/index.html for docs on the automation system
    "-TestExit=`"Automation Test Queue Empty`"", # When to close the editor
    "-ReportOutputPath=`"$($output_dir_absolute)`"", # Output folder for test results. If it doesn't exist, gets created. If it does, all contents get deleted before new results get placed there.
    "-ABSLOG=`"$($log_file_path)`"", # Sets the path for the log file produced during this run.
    "-nopause", # Close the unreal log window automatically on exit
    "-nosplash", # No splash screen
    "-unattended", # Disable anything requiring user feedback
    "-nullRHI", # Hard to find documentation for, but seems to indicate that we want something akin to a headless (i.e. no UI / windowing) editor
    "-OverrideSpatialNetworking=$run_with_spatial" # A parameter to switch beetween different networking implementations
)

Write-Output "Running $($ue_path_absolute) $($cmd_args_list)"

$run_tests_proc = Start-Process $ue_path_absolute -PassThru -NoNewWindow -ArgumentList $cmd_args_list
try {
    # Give the Unreal Editor 30 minutes to run the tests, otherwise kill it
    # This is so we can get some logs out of it, before we are cancelled by buildkite
    Wait-Process -Timeout 1800 -InputObject $run_tests_proc
}
catch {
    buildkite-agent artifact upload "$log_file_path" # If the tests timed out, upload the log and throw an error
    throw $_
}
