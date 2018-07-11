
$fileshare="\\filesharing2\files"
$version="v0.96"
$rootPath=[System.Io.Path]::GetFullPath("$env:HOMEDRIVE:\tools\fastbuild")

If (-NOT ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator))
{
    Write-Host "Please run from an Adminstrator command prompt."
    exit 1
}

$fastbuildProcess=Get-Process -Name "FBuildWorker.exe.copy" -ErrorAction SilentlyContinue

if ($fastbuildProcess) {
    $name=$fastbuildProcess.FileName
    Write-Host "$name is currently running. Stopping it."
    Stop-Process -Id $fastbuildProcess.Id
}

Import-Module BitsTransfer
Add-Type -AssemblyName System.IO.Compression.FileSystem

if (Test-Path $rootPath) {
    [System.IO.Directory]::Delete($rootPath, $true)
}
[System.IO.Directory]::CreateDirectory($rootPath)

$fbuildPath=[System.IO.Path]::GetFullPath("$rootPath\FBuild.exe")
$fbuildWorkerPath=[System.IO.Path]::GetFullPath("$rootPath\FBuildWorker.exe")

if (-not (Test-Path "$rootPath\fastbuild-$version.zip")) {
    Start-BitsTransfer -Source "http://www.fastbuild.org/downloads/$version/FASTBuild-Windows-x64-$version.zip" -Destination "$rootPath\fastbuild-$version.zip" -ErrorAction Stop

    [System.IO.Compression.ZipFile]::ExtractToDirectory("$rootPath\fastbuild-$version.zip", $rootPath)
}

# FBuildWorker launches a detached sub-process to avoid losing data when builds are canceled.
# This copy is the one that accesses the network, so that's what we add to the firewall.
$fbuildWorkerCopyPath = [System.IO.Path]::GetFullPath("$rootPath\FBuildWorker.exe.copy")

$port="31264"
Remove-NetFirewallRule -Group "FASTBuild" `
    -ErrorAction SilentlyContinue

New-NetFirewallRule -Name "FBuild" `
    -DisplayName "FBuild" `
    -Group "FASTBuild" `
    -Description "Allow distributed compiling via the FASTBuild driver" `
    -Enabled True `
    -Direction Outbound `
    -Protocol TCP `
    -LocalPort $port `
    -Program "$fbuildPath" `
    -Action Allow `
    -ErrorAction Stop

New-NetFirewallRule -Name "FBuildWorker" `
    -DisplayName "FBuildWorker" `
    -Group "FASTBuild" `
    -Description "Allow distributed compiling via the FASTBuild worker" `
    -Enabled True `
    -Direction Outbound `
    -Protocol TCP `
    -LocalPort $port `
    -Program "$fbuildWorkerCopyPath" `
    -Action Allow `
    -ErrorAction Stop

New-ItemProperty `
    -Path HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Run\ `
    -Name "FBuildWorker" `
    -Value """$fbuildWorkerPath""" -Force

[System.Environment]::SetEnvironmentVariable("FASTBUILD_EXE_PATH", $fbuildPath, [System.EnvironmentVariableTarget]::User)
[System.Environment]::SetEnvironmentVariable("FASTBUILD_CACHE_PATH", "$fileshare\FastBuild_Cache", [System.EnvironmentVariableTarget]::User)
[System.Environment]::SetEnvironmentVariable("FASTBUILD_BROKERAGE_PATH", "$fileshare\FastBuild_Brokerage", [System.EnvironmentVariableTarget]::User)
[System.Environment]::SetEnvironmentVariable("FASTBUILD_CACHE_MODE", "rw", [System.EnvironmentVariableTarget]::User)
