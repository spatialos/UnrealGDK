@echo off

rem Input args: BuildName, Platform, BuildConfiguration, uproject 

set IsProjectPlugin=""

rem If we are running as a project plugin then this will be the path to the spatial directory.
if exist "%~dp0..\..\..\..\..\..\spatial" (
	echo Building as project plugin
	set SpatialDir="%~dp0..\..\..\..\..\..\spatial"
	set BUILD_EXE_PATH="Plugins\UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\Build.exe"
	goto Build
)


:BuildAsEnginePlugin
rem If we are running as an Engine plugin then find the project path and spatial path.
echo Building as engine plugin

rem Grab the project path from the .uproject file.
set uproject=%4
for %%i IN (%uproject%) DO (
	set ProjectDir=%%~pi
)

set SpatialDir=%ProjectDir%spatial

echo Using project directory: %ProjectDir%
echo Using spatial directory: %SpatialDir%

rem We have found the spatial directory for the project!
if not exist %SpatialDir% (
	goto: SpatialNotExist
)

rem Path to the SpatialGDK build tool.
set BUILD_EXE_PATH="%~dp0..\..\Binaries\ThirdParty\Improbable\Programs\Build.exe"


:Build

rem Build the spatial worker configs
pushd "%SpatialDir%"
spatial worker build build-config
popd

rem Build Unreal project using the SpatialGDK build tool

rem This little bit allows using a relative path to the .uproject but only for project plugins.........
pushd "%~dp0..\..\..\..\..\"

if not exist %BUILD_EXE_PATH% (
	echo Error: Build executable not found! Please run Setup.bat in your UnrealGDK root to generate it.
	exit /b 1
)

%BUILD_EXE_PATH% %*
popd
exit /b %ERRORLEVEL%


:SpatialNotExist

echo Could not find the projects 'spatial' directory! Please ensure your input arguments are correct.
exit /b 1