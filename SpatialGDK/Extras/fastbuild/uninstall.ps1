param (
    [switch]$service=$false
)

$rootPath=[System.IO.Path]::GetFullPath("$env:HOMEDRIVE:\tools\fastbuild")
# NOTE: When updating '$version', ensure you also update the '$brokerageFile' path to be correct.
# This will involve replacing brokerage number after "main". e.g. "19.windows" -> "20.windows". Check your brokerage for the correct version.
$version="v0.98"

If (-NOT ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Host "Please run from an Adminstrator command prompt."
    exit 1
}

Write-Host "Removing from firewall..."
Remove-NetFirewallRule -Group "FASTBuild" `
    -ErrorAction SilentlyContinue

if ($service) {
    $serviceName="FASTBuildWorker-$version"

    if (Get-Service -Name $serviceName -ErrorAction SilentlyContinue) {
        Write-Host "Stopping $serviceName service..."
        Start-Process nssm.exe "stop",$serviceName -Wait -NoNewWindow

        Write-Host "Removing $serviceName service..."
        Start-Process nssm.exe "remove",$serviceName,"confirm" -Wait  -NoNewWindow
    }
} else {
    Write-Host "Removing from startup group..."
    Remove-ItemProperty `
        -Path HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Run\ `
        -Name "FBuildWorker" `
        -ErrorAction SilentlyContinue

    Write-Host "Stopping process..."
    Stop-Process -Name "FBuildWorker.exe" -ErrorAction SilentlyContinue
    Stop-Process -Name "FBuildWorker.exe.copy" -ErrorAction SilentlyContinue
}

# Manually remove marker file from the brokerage, since it's not possible to cleanly exit FBuildWorker.exe
$brokerageFile = [System.IO.Path]::Combine($env:FASTBUILD_BROKERAGE_PATH, "main", "19.windows", [System.Net.Dns]::GetHostName())
if (Test-Path $brokerageFile) {
    Write-Host "Removing from brokerage..."
    [System.IO.File]::Delete($brokerageFile)
}

Write-Host "Cleaning environment..."
[System.Environment]::SetEnvironmentVariable("FASTBUILD_EXE_PATH", $null, [System.EnvironmentVariableTarget]::Machine)
[System.Environment]::SetEnvironmentVariable("FASTBUILD_CACHE_PATH", $null, [System.EnvironmentVariableTarget]::Machine)
[System.Environment]::SetEnvironmentVariable("FASTBUILD_BROKERAGE_PATH", $null, [System.EnvironmentVariableTarget]::Machine)
[System.Environment]::SetEnvironmentVariable("FASTBUILD_CACHE_MODE", $null, [System.EnvironmentVariableTarget]::Machine)

if (Test-Path $rootPath) {
    Write-Host "Removing $rootPath..."
    [System.IO.Directory]::Delete($rootPath, $true)
}

Write-Host "Finished!" -ForegroundColor Green
