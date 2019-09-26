param(
    [string] $ue_path,
    [string] $gdk_home = (get-item "$($PSScriptRoot)").parent.FullName, ## The root of the UnrealGDK repo
    [string] $uproject_path,
    [string] $output_dir,
    [string] $log_file_name
)

function Force-Resolve-Path {
    param (
        [string] $path
    )
    return $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($path)
}

$ue_path_absolute = Force-Resolve-Path $ue_path
$uproject_path_absolute = Force-Resolve-Path $uproject_path
$output_dir_absolute = Force-Resolve-Path $output_dir

Write-Output Get-ChildItem $ue_path_absolute
Write-Output Get-ChildItem "$($ue_path_absolute)\Samples"
Write-Output Get-ChildItem "$($ue_path_absolute)\Samples\UnrealGDKShooterGame"

Write-Output $ue_path_absolute
Write-Output "$(Test-Path $ue_path_absolute)"

Write-Output $uproject_path_absolute
Write-Output "$(Test-Path $uproject_path_absolute)"

Write-Output $output_dir_absolute
Write-Output "$(Test-Path $output_dir_absolute)"

$cmd_list = @( `
    "`"$($uproject_path_absolute)`"",
    "-ExecCmds=`"automation runtests SpatialGDK; quit`"", `
    "-TestExit=`"Automation Test Queue Empty`"", `
    "-ReportOutputPath=`"$($output_dir_absolute)`"", `
    "Log=`"$($log_file_name)`"", ` # set the log file name. Its location is TODO. the lack of "-" is correct, -Log is a flag and doesn't set the file name
    "-nopause", `
    "-nosplash", `
    "-unattended", `
    "-nullRHI"
)

Write-Output "Running $($ue_path_absolute) $($cmd_list)"

# $test_proc = Start-Process -Wait -PassThru -NoNewWindow $ue_path_absolute -ArgumentList $cmd_list

# Write-Output "Exited with code: $($test_proc.ExitCode)" # can't find an indication of what the exit codes actually mean, so not relying on them

$results_path = Join-Path -Path $output_dir_absolute -ChildPath "index.json"
$results_json = Get-Content $results_path -Raw

# Write-Output $results_json

$results_obj = ConvertFrom-Json $results_json
Write-Output $results_obj

if ($results_obj.failed -ne 0) {
    Write-Output "$($results_obj.failed) tests failed."
    Throw "Some tests failed"
}
