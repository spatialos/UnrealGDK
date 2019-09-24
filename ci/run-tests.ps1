param(
    [string] $ue_path,
    [string] $gdk_home = (get-item "$($PSScriptRoot)").parent.FullName, ## The root of the UnrealGDK repo
    [string] $uproject_path,
    [string] $output_dir,
    [string] $log_path
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
$log_path_absolute = Force-Resolve-Path $log_path

$cmd_list = @( `
    # "`"$($uproject_path_absolute)`"",
    "-ExecCmds=`"automation runtests now SpatialGDK; quit`"", `
    "-nopause", `
    "-nosplash", `
    "-Unattended", `
    "-NullRHI", `
    "-TestExit=`"Automation Test Queue Empty`"", `
    "-ReportOutputPath=`"$($output_dir_absolute)`"", `
    "Log=`"$($log_path_absolute)`""
)

Write-Host "Running $($ue_path_absolute) $($cmd_list)"

$test_proc = Start-Process -Wait -PassThru -NoNewWindow $ue_path_absolute -ArgumentList $cmd_list

Write-Host "Exited with code: $($test_proc.ExitCode)"

if ($test_proc.ExitCode -ne 0) {
    Write-Host "Exited with code: $($test_proc.ExitCode)"
}

# .\Engine\Binaries\Win64\UE4Editor.exe "C:\Users\michelefabris\git\UnrealGDKExampleProject\Game\GDKShooter.uproject" 
# -ExecCmds="Automation runtests SpatialGDK" -log  -Unattended -NullRHI 
# -TestExit="Automation Test Queue Empty" 
# -ReportOutputPath="C:\Users\michelefabris\GdkTestReport" -Log -Log=RunTests.log
