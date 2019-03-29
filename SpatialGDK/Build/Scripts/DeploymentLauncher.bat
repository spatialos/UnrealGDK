@echo off

pushd "%~dp0..\..\..\..\..\"

set DEPLOYMENT_LAUNCHER_EXE_PATH="Plugins\UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\DeploymentLauncher\DeploymentLauncher.exe"

if not exist %DEPLOYMENT_LAUNCHER_EXE_PATH% (
	echo Error: Deployment launcher executable not found! Please run Setup.bat in your UnrealGDK root to generate it.
	exit /b 1
)

%DEPLOYMENT_LAUNCHER_EXE_PATH% %*

popd

pause

exit /b %ERRORLEVEL%
