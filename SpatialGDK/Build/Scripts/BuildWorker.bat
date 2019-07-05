@echo off

rem Input args: BuildName : Platform : 

set IsProjectPlugin=""

rem If we are running as a project plugin then this will be the path to the spatial directory.
if exist "%~dp0..\..\..\..\..\..\spatial" (
	goto BuildAsProjectPlugin
)

rem If we are running as an Engine plugin then this is more complicated....
:BuildAsEnginePlugin
echo Building as engine plugin

rem We need to get the spatial directory somehow from this location.
rem Use the path of the uproject to find the spatial directory.
rem Or we can use an input variable and update the docs.

echo 0: %0
echo 1: %1
echo 2: %2
echo 3: %3
echo 4: %4
echo 5: %5
echo 6: %6

echo dir: %~dp0
echo dir1: %~dp1
echo dir2: %~dp2
echo cd: !cd!

rem TODO: Get the filepath of the project with this little hacky code, then use this to get the spatial directory and whatever else we need.
set uproject=%4
for %%i IN (%uproject%) DO (
rem ECHO filedrive=%%~di
rem ECHO filepath=%%~pi
set ProjectDir=%%~pi
rem ECHO filename=%%~ni
rem ECHO fileextension=%%~xi
)

echo Project Directory is: %ProjectDir%

set SpatialDir=%ProjectDir%spatial

echo %SpatialDir%

pushd "%SpatialDir%"
spatial worker build build-config
popd


rem TODO: Final thing is get this working with Engine Build Exe path.
set BUILD_EXE_PATH=""

goto Build


:BuildAsProjectPlugin
echo Building as project plugin

pushd "%~dp0..\..\..\..\..\..\spatial"
spatial worker build build-config
popd

rem Build Unreal project
pushd "%~dp0..\..\..\..\..\"

set BUILD_EXE_PATH="Plugins\UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\Build.exe"

goto Build

:Build

if not exist %BUILD_EXE_PATH% (
	echo Error: Build executable not found! Please run Setup.bat in your UnrealGDK root to generate it.
	exit /b 1
)

%BUILD_EXE_PATH% %*

popd

exit /b %ERRORLEVEL%
