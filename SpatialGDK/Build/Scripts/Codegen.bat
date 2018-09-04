@echo off

pushd "%~dp0..\..\"

set CODEGEN_PATH="Binaries\ThirdParty\Improbable\Programs\Codegen.exe"

if not exist %CODEGEN_PATH% (
	echo Error: Codegen executable not found! Please run BuildGDK.bat in your SpatialGDK directory to generate it.
	exit /b 1
)

%CODEGEN_PATH% %*

popd

exit /b %ERRORLEVEL%
