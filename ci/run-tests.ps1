param(
    [string] $unreal_editor_path,
    [string] $uproject_path,
    [string] $test_repo_path,
    [string] $log_file_path,
    [string] $test_repo_map,
    [string] $report_output_path,
    [string] $tests_path = "SpatialGDK",
    [string] $additional_gdk_options = "",
    [string] $additional_gdk_editor_options = "",
    [bool]   $run_with_spatial = $False,
    [string] $additional_cmd_line_args = ""
)

# This resolves a path to be absolute, without actually reading the filesystem.
# This means it works even when the indicated path does not exist, as opposed to the Resolve-Path cmdlet
function Force-ResolvePath {
    param (
        [string] $path
    )
    return $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($path)
}

function Parse-UnrealOptions {
    param (
        [string] $raw_options,
        [string] $category
    )
    $options_arr = $raw_options.Split(";", [System.StringSplitOptions]::RemoveEmptyEntries)
    $options_arr = $options_arr | ForEach-Object { "${category}:$_" }
    $options_result = $options_arr -Join ","
    return $options_result
}

. "$PSScriptRoot\common.ps1"

if ($run_with_spatial) {
    # Generate schema and snapshots
    Write-Output "Generating snapshot and schema for testing project"
    Start-Process "$unreal_editor_path" -Wait -PassThru -NoNewWindow -ArgumentList @(`
        "$uproject_path", `
        "-SkipShaderCompile", # Skip shader compilation
        "-nopause", # Close the unreal log window automatically on exit
        "-nosplash", # No splash screen
        "-unattended", # Disable anything requiring user feedback
        "-nullRHI", # Hard to find documentation for, but seems to indicate that we want something akin to a headless (i.e. no UI / windowing) editor
        "-run=CookAndGenerateSchema", # Run the commandlet
        "-cookall", # Make sure it runs for all maps (and other things)
        "-targetplatform=LinuxServer"
    )
    
    Start-Process "$unreal_editor_path" -Wait -PassThru -NoNewWindow -ArgumentList @(`
        "$uproject_path", `
        "-NoShaderCompile", # Prevent shader compilation
        "-nopause", # Close the unreal log window automatically on exit
        "-nosplash", # No splash screen
        "-unattended", # Disable anything requiring user feedback
        "-nullRHI", # Hard to find documentation for, but seems to indicate that we want something akin to a headless (i.e. no UI / windowing) editor
        "-run=GenerateSnapshot", # Run the commandlet
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

$additional_gdk_options = Parse-UnrealOptions "$additional_gdk_options" "[/Script/SpatialGDK.SpatialGDKSettings]"
$additional_gdk_editor_options = Parse-UnrealOptions "$additional_gdk_editor_options" "[/Script/SpatialGDKEditor.SpatialGDKEditorSettings]"

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
    "-stdout", # Print to output
    "-ini:SpatialGDKSettings:$additional_gdk_options" # Pass changes to SpatialGDK configuration files from above
    "-ini:SpatialGDKEditorSettings:$additional_gdk_editor_options" # Pass changes to SpatialGDKEditor configuration files from above
    "-OverrideSpatialNetworking=$run_with_spatial" # A parameter to switch beetween different networking implementations
)

if($additional_cmd_line_args -ne "") {
    $cmd_args_list += "$additional_cmd_line_args" # Any additional command line arguments the user wants to pass in
}

Write-Output "Running $($ue_path_absolute) $($cmd_args_list)"

$run_tests_proc = Start-Process $ue_path_absolute -PassThru -NoNewWindow -ArgumentList $cmd_args_list
try {
    # Give the Unreal Editor 30 minutes to run the tests, otherwise kill it
    # This is so we can get some logs out of it, before we are cancelled by buildkite
    Wait-Process -Timeout 1800 -InputObject $run_tests_proc
    # If the Editor crashes, these processes can stay lingering and prevent the job from ever timing out
    Stop-Runtime
}
catch {
    Stop-Process -Force -InputObject $run_tests_proc # kill the dangling process
    buildkite-agent artifact upload "$log_file_path" # If the tests timed out, upload the log and throw an error
    # Looks like BuildKite doesn't like this failure and a dangling runtime will prevent the job from ever timing out
    Stop-Runtime
    throw $_
}
