param (
    [switch]$service=$false
)

Import-Module BitsTransfer
Add-Type -AssemblyName System.IO.Compression.FileSystem

$fileshare="\\lonv-file-01"
$version="v0.98"
$rootPath=[System.Io.Path]::GetFullPath("$env:HOMEDRIVE:\tools\fastbuild")

If (-NOT ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Host "Please run from an Adminstrator command prompt." -ForegroundColor Red
    exit 1
}

if (Test-Path $rootPath) {
    Write-Host "FASTBuild is already installed at $rootPath. Please uninstall it first." -ForegroundColor Yellow
    exit 1
}
[System.IO.Directory]::CreateDirectory($rootPath) | Out-Null

$fbuildPath=[System.IO.Path]::GetFullPath("$rootPath\FBuild.exe")
$fbuildWorkerPath=[System.IO.Path]::GetFullPath("$rootPath\FBuildWorker.exe")

Write-Host "Downloading FASTBuild..."
if (-not (Test-Path "$rootPath\fastbuild-$version.zip")) {
    Start-BitsTransfer -Source "http://www.fastbuild.org/downloads/$version/FASTBuild-Windows-x64-$version.zip" -Destination "$rootPath\fastbuild-$version.zip" -ErrorAction Stop | Out-Null

    [System.IO.Compression.ZipFile]::ExtractToDirectory("$rootPath\fastbuild-$version.zip", $rootPath)
}

# Ensure that all logged-in users can access the install folder.
$acl = Get-ACL -Path $rootPath
$ar = New-Object System.Security.AccessControl.FileSystemAccessRule( `
    "BUILTIN\Users", "ReadAndExecute", "Allow")
    $acl.AddAccessRule($ar)
$ar = New-Object System.Security.AccessControl.FileSystemAccessRule( `
	"BUILTIN\Users", "Write", "Allow")
$acl.AddAccessRule($ar)
Set-Acl -Path $rootPath -AclObject $acl | Out-Null

# FBuildWorker launches a detached sub-process to avoid losing data when builds are canceled.
# This copy is the one that accesses the network, so that's what we add to the firewall.
$fbuildWorkerCopyPath = [System.IO.Path]::GetFullPath("$rootPath\FBuildWorker.exe.copy")

$port="31264"

Write-Host "Adding to firewall..."
New-NetFirewallRule -Name "FBuild (Outbound)" `
    -DisplayName "FBuild (Outbound)" `
    -Group "FASTBuild" `
    -Description "Allow distributed compiling via the FASTBuild driver" `
    -Enabled True `
    -Direction Outbound `
    -Protocol TCP `
    -LocalPort $port `
    -Program "$fbuildPath" `
    -Action Allow `
    -ErrorAction Stop | Out-Null

New-NetFirewallRule -Name "FBuildWorker (Outbound)" `
    -DisplayName "FBuildWorker (Outbound)" `
    -Group "FASTBuild" `
    -Description "Allow distributed compiling via the FASTBuild worker" `
    -Enabled True `
    -Direction Outbound `
    -Protocol TCP `
    -LocalPort $port `
    -Program "$fbuildWorkerCopyPath" `
    -Action Allow `
    -ErrorAction Stop | Out-Null

New-NetFirewallRule -Name "FBuildWorker (Inbound)" `
    -DisplayName "FBuildWorker (Inbound)" `
    -Group "FASTBuild" `
    -Description "Allow distributed compiling via the FASTBuild worker" `
    -Enabled True `
    -Direction Inbound `
    -Protocol TCP `
    -LocalPort $port `
    -Program "$fbuildWorkerCopyPath" `
    -Action Allow `
    -ErrorAction Stop | Out-Null

New-NetFirewallRule -Name "FBuildWorker.Copy (Outbound)" `
    -DisplayName "FBuildWorker.Copy (Outbound)" `
    -Group "FASTBuild" `
    -Description "Allow distributed compiling via the FASTBuild worker" `
    -Enabled True `
    -Direction Outbound `
    -Protocol TCP `
    -LocalPort $port `
    -Program "$fbuildWorkerPath" `
    -Action Allow `
    -ErrorAction Stop | Out-Null

New-NetFirewallRule -Name "FBuildWorker.Copy (Inbound)" `
    -DisplayName "FBuildWorker.Copy (Inbound)" `
    -Group "FASTBuild" `
    -Description "Allow distributed compiling via the FASTBuild worker" `
    -Enabled True `
    -Direction Inbound `
    -Protocol TCP `
    -LocalPort $port `
    -Program "$fbuildWorkerPath" `
    -Action Allow `
    -ErrorAction Stop | Out-Null

Write-Host "Setting environment variables..."
[System.Environment]::SetEnvironmentVariable("FASTBUILD_EXE_PATH", $fbuildPath, [System.EnvironmentVariableTarget]::Machine)
[System.Environment]::SetEnvironmentVariable("FASTBUILD_CACHE_PATH", "$fileshare\fastbuild\Cache", [System.EnvironmentVariableTarget]::Machine)
[System.Environment]::SetEnvironmentVariable("FASTBUILD_BROKERAGE_PATH", "$fileshare\fastbuild\Brokerage", [System.EnvironmentVariableTarget]::Machine)
[System.Environment]::SetEnvironmentVariable("FASTBUILD_CACHE_MODE", "rw", [System.EnvironmentVariableTarget]::Machine)

Write-Host "Setting up FASTBuild to startup on login..."

if ($service) {
    # Install a service manager to wrap FBuildWorker so it can run as a service.
    # http://nssm.cc
    Start-Process "choco" "install","nssm","-y","--version=2.24.101.20180116" -Wait -ErrorAction Stop -NoNewWindow
    refreshenv

    $serviceName="FASTBuildWorker-$version"

    Write-Host "Installing $serviceName service..."

    Start-Process nssm.exe "install",$serviceName,"$fbuildWorkerPath","-console","-nosubprocess","-cpus=100%" -Wait -ErrorAction Stop -NoNewWindow
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to install $serviceName"
    }
    Start-Process nssm.exe "set",$serviceName,"Start","SERVICE_DELAYED_AUTO_START"-Wait -ErrorAction Stop -NoNewWindow
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to set $serviceName Start type"
    }
    Start-Service -Name $serviceName
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to start $serviceName"
    }
} else {
    Write-Host "Adding to startup group..."
    # Set to run on startup.
    New-ItemProperty `
        -Path HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Run\ `
        -Name "FBuildWorker" `
        -Value """$fbuildWorkerPath""" -Force | Out-Null

    Start-Process "$fbuildWorkerPath" -ErrorAction Stop
}

Write-Host "Finished!" -ForegroundColor Green
