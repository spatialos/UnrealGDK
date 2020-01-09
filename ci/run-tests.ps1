param(
    [string] $unreal_editor_path,
    [string] $uproject_path,
    [string] $test_repo_path,
    [string] $log_file_path,
    [string] $test_repo_map,
    [string] $report_output_path,
    [string] $tests_path = "SpatialGDK",
    [bool] $run_with_spatial = $false
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
    Echo "Generating snapshot and schema for testing project"
    $commandlet_process = Start-Process "$unreal_editor_path" -Wait -PassThru -NoNewWindow -ArgumentList @(`
        "$uproject_path", `
        "-NoShaderCompile", ` # Prevent shader compilation
        "-nopause", ` # Close the unreal log window automatically on exit
        "-nosplash", ` # No splash screen
        "-unattended", ` # Disable anything requiring user feedback
        "-nullRHI", ` # Hard to find documentation for, but seems to indicate that we want something akin to a headless (i.e. no UI / windowing) editor
        "-run=GenerateSchemaAndSnapshots", ` # Run the commandlet
        "-MapPaths=`"$test_repo_map`"" ` # Which maps to run the commandlet for
    )

    # Create the default snapshot
    Copy-Item -Force `
        -Path "$test_repo_path\spatial\snapshots\$test_repo_map.snapshot" `
        -Destination "$test_repo_path\spatial\snapshots\default.snapshot"


    pushd "$test_repo_path\spatial"
        $build_configs_process = Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
            "build", `
            "build-config"
        )

        if ($build_configs_process.ExitCode -ne 0) {
            Write-Log "Failed to build worker configurations for the project. Error: $($build_configs_process.ExitCode)"
            Throw "Failed to build worker configurations"
        }

        $spatial_process = Start-Process "spatial" -PassThru -NoNewWindow -ArgumentList @("local", "launch")
    popd

    Start-Sleep -s 10
}

# Create the TestResults directory if it does not exist, for storing results
New-Item -Path "$PSScriptRoot" -Name "$report_output_path" -ItemType "directory" -ErrorAction SilentlyContinue
$output_dir = "$PSScriptRoot\$report_output_path"

# We want absolute paths since paths given to the unreal editor are interpreted as relative to the UE4Editor binary
# Absolute paths are more reliable
$ue_path_absolute = Force-ResolvePath $unreal_editor_path
$uproject_path_absolute = Force-ResolvePath $uproject_path
$output_dir_absolute = Force-ResolvePath $output_dir

$cmd_args_list = @( `
    "`"$uproject_path_absolute`"", ` # We need some project to run tests in, but for unit tests the exact project shouldn't matter
    "`"$test_repo_map`"", ` # The map to run tests in
    "-ExecCmds=`"Automation RunTests $tests_path; Quit`"", ` # Run all tests. See https://docs.unrealengine.com/en-US/Programming/Automation/index.html for docs on the automation system
    "-TestExit=`"Automation Test Queue Empty`"", ` # When to close the editor
    "-ReportOutputPath=`"$($output_dir_absolute)`"", ` # Output folder for test results. If it doesn't exist, gets created. If it does, all contents get deleted before new results get placed there.
    "-ABSLOG=`"$($log_file_path)`"", ` # Sets the path for the log file produced during this run.
    "-nopause", ` # Close the unreal log window automatically on exit
    "-nosplash", ` # No splash screen
    "-unattended", ` # Disable anything requiring user feedback
    "-nullRHI" # Hard to find documentation for, but seems to indicate that we want something akin to a headless (i.e. no UI / windowing) editor
)

if($run_with_spatial) {
    $cmd_args_list += "-OverrideSpatialNetworking=$run_with_spatial" # A parameter to switch beetween different networking implementations
}

Echo "Running $($ue_path_absolute) $($cmd_args_list)"

$run_tests_proc = Start-Process $ue_path_absolute -PassThru -NoNewWindow -ArgumentList $cmd_args_list
Start-Sleep -s 10
$tail_proc = Start-Process "powershell" -PassThru -ArgumentList @("Get-Content", $log_file_path,  "-Wait")
Wait-Process -Id (Get-Process -InputObject $run_tests_proc).id
Stop-Process -InputObject $tail_proc

Stop-Process -InputObject $spatial_process
