@echo off

set DEPLOYMENT_LAUNCHER_EXE_PATH="%~dp0..\..\Binaries\ThirdParty\Improbable\Programs\DeploymentLauncher\DeploymentLauncher.exe"

if not exist %DEPLOYMENT_LAUNCHER_EXE_PATH% (
	echo Error: Deployment launcher executable not found! Please run Setup.bat in your UnrealGDK root to generate it.
	pause
	exit /b 1
)

%DEPLOYMENT_LAUNCHER_EXE_PATH% %*

pause

exit /b %ERRORLEVEL%
