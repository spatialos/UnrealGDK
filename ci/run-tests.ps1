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

$absolute_uproject = Force-Resolve-Path $uproject_path
$absolute_output_dir = Force-Resolve-Path $output_dir
$absoute_log_path = Force-Resolve-Path $log_path

$cmd_list = @( `
    "`"$($absolute_uproject)`"",
    "-ExecCmds=`"Automation runtests SpatialGDK`"", `
    "-Unattended", `
    "-NullRHI", `
    "-TestExit=`"Automation Test Queue Empty`"", `
    "-ReportOutputPath=`"$($absolute_output_dir)`"", `
    "-Log=`"$($absoute_log_path)`""
)

Write-Host "Running UE4Editor.exe with: $($cmd_list)"

$test_proc = Start-Process -Wait -PassThru -NoNewWindow $ue_path -ArgumentList @( `
    "`"$($absolute_uproject)`"",
    "-ExecCmds=`"Automation runtests SpatialGDK`"", `
    "-Unattended", `
    "-NullRHI", `
    "-TestExit=`"Automation Test Queue Empty`"", `
    "-ReportOutputPath=`"$($absolute_output_dir)`"", `
    "-Log=`"$($absoute_log_path)`""
)

# $test_proc = Start-Process -Wait -PassThru -NoNewWindow $ue_path -ArgumentList @( `
#     "`"$($uproject_path)`""
# )

if ($test_proc.ExitCode -ne 0) {
    Write-Host "Exited with code: $($test_proc.ExitCode)"
}

# .\Engine\Binaries\Win64\UE4Editor.exe "C:\Users\michelefabris\git\UnrealGDKExampleProject\Game\GDKShooter.uproject" 
# -ExecCmds="Automation runtests SpatialGDK" -log  -Unattended -NullRHI 
# -TestExit="Automation Test Queue Empty" 
# -ReportOutputPath="C:\Users\michelefabris\GdkTestReport" -Log -Log=RunTests.log
