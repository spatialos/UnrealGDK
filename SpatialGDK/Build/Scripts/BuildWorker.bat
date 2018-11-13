@echo off

pushd "%~dp0..\..\..\..\..\..\spatial"

spatial worker build build-config

popd

pushd "%~dp0..\..\..\..\..\"

set BUILD_EXE_PATH="Plugins\UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\Build.exe"

if not exist %BUILD_EXE_PATH% (
	echo Error: Build executable not found! Please run Setup.bat in your UnrealGDK root to generate it.
	exit /b 1
)

%BUILD_EXE_PATH% %*

popd

exit /b %ERRORLEVEL%
