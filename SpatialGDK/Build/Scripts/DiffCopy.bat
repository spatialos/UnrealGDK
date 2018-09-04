@echo off

pushd "%~dp0..\..\"

set DIFFCOPY_PATH="Binaries\ThirdParty\Improbable\Programs\DiffCopy.exe"

if not exist %DIFFCOPY_PATH% (
	echo Error: DiffCopy executable not found! Please run BuildGDK.bat in your SpatialGDK directory to generate it.
	exit /b 1
)

%DIFFCOPY_PATH% %*

popd

exit /b %ERRORLEVEL%
