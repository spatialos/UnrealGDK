@echo off

pushd "%~dp0..\..\"

call "Build\Scripts\BuildWorkerConfig.bat"

popd

pushd "%~dp0..\..\..\..\..\"

set BUILD_EXE_PATH="Plugins\UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\Build.exe"

if not exist %BUILD_EXE_PATH% (
	echo Error: Build executable not found! Please run BuildGDK.bat in your SpatialGDK directory to generate it.
	exit /b 1
)

%BUILD_EXE_PATH% %*

popd

exit /b %ERRORLEVEL%
