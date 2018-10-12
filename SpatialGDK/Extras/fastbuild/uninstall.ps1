param (
    [switch]$service=$false
)

$rootPath=[System.Io.Path]::GetFullPath("$env:HOMEDRIVE:\tools\fastbuild")
$version="v0.96"

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

Write-Host "Cleaning environment..."
[System.Environment]::SetEnvironmentVariable("FASTBUILD_EXE_PATH", $null, [System.EnvironmentVariableTarget]::Machine)
[System.Environment]::SetEnvironmentVariable("FASTBUILD_CACHE_PATH", $null, [System.EnvironmentVariableTarget]::Machine)
[System.Environment]::SetEnvironmentVariable("FASTBUILD_BROKERAGE_PATH", $null, [System.EnvironmentVariableTarget]::Machine)
[System.Environment]::SetEnvironmentVariable("FASTBUILD_CACHE_MODE", $null, [System.EnvironmentVariableTarget]::Machine)

refreshenv

if (Test-Path $rootPath) {
    Write-Host "Removing $rootPath..."
    [System.IO.Directory]::Delete($rootPath, $true)
}

Write-Host "Finished!" -ForegroundColor Green
