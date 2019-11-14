param(
    [string] $unreal_editor_path,
    [string] $uproject_path,
    [string] $output_dir,
    [string] $log_file_path,
    [string] $test_repo_map
)

# This resolves a path to be absolute, without actually reading the filesystem.
# This means it works even when the indicated path does not exist, as opposed to the Resolve-Path cmdlet
function Force-ResolvePath {
    param (
        [string] $path
    )
    return $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($path)
}

# We want absolute paths since paths given to the unreal editor are interpreted as relative to the UE4Editor binary
# Absolute paths are more reliable
$ue_path_absolute = Force-ResolvePath $unreal_editor_path
$uproject_path_absolute = Force-ResolvePath $uproject_path
$output_dir_absolute = Force-ResolvePath $output_dir

$cmd_args_list = @( `
    "`"$uproject_path_absolute`"", ` # We need some project to run tests in, but for unit tests the exact project shouldn't matter
    "`"$test_repo_map`"", ` # The map to run tests in
    "-ExecCmds=`"Automation RunTests SpatialGDK; Quit`"", ` # Run all tests. See https://docs.unrealengine.com/en-US/Programming/Automation/index.html for docs on the automation system
    "-TestExit=`"Automation Test Queue Empty`"", ` # When to close the editor
    "-ReportOutputPath=`"$($output_dir_absolute)`"", ` # Output folder for test results. If it doesn't exist, gets created. If it does, all contents get deleted before new results get placed there.
    "-ABSLOG=`"$($log_file_path)`"", ` # Sets the path for the log file produced during this run.
    "-nopause", ` # Close the unreal log window automatically on exit
    "-nosplash", ` # No splash screen
    "-unattended", ` # Disable anything requiring user feedback
    "-nullRHI" # Hard to find documentation for, but seems to indicate that we want something akin to a headless (i.e. no UI / windowing) editor
)

Write-Log "Running $($ue_path_absolute) $($cmd_args_list)"

$run_tests_proc = Start-Process -PassThru -NoNewWindow $ue_path_absolute -ArgumentList $cmd_args_list
Wait-Process -Id (Get-Process -InputObject $run_tests_proc).id

# Workaround for UNR-2156 and UNR-2076, where spatiald / runtime processes sometimes never close, or where runtimes are orphaned
# Clean up any spatiald and java (i.e. runtime) processes that may not have been shut down
Start-Process spatial "service","stop" -Wait -ErrorAction Stop -NoNewWindow
Stop-Process -Name "java" -Force -ErrorAction SilentlyContinue
