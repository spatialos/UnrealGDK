@echo off

if [%4] == [] goto :MissingParams

rem Try and build as a project plugin first, check for a project plugin structure.
set UNREAL_PROJECT_DIR="%~dp0..\..\..\..\..\"
set FOUND_UPROJECT=""

for /f "delims=" %%A in (' powershell -Command "(Get-ChildItem -Path "%UNREAL_PROJECT_DIR%" *.uproject).FullName" ') do set FOUND_UPROJECT="%%A"

if %FOUND_UPROJECT%=="" (
	goto :BuildAsEnginePlugin
)

rem If we are running as a project plugin then this will be the path to the spatial directory.
echo Building as project plugin
set SPATIAL_DIR="%~dp0..\..\..\..\..\..\spatial"
set BUILD_EXE_PATH="Plugins\UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\Build.exe"
goto :Build


:BuildAsEnginePlugin
rem If we are running as an Engine plugin then we need a full path to the .uproject file!
set UPROJECT=%4

if not exist %UPROJECT% (
	echo To use BuildWorker.bat with an Engine plugin installation you must provide the full path to your .uproject file.
	exit /b 1
)

rem Grab the project path from the .uproject file.
for %%i in (%UPROJECT%) do (
	rem file drive + file directory
	set UNREAL_PROJECT_DIR="%%~di%%~pi"
)

rem Path to the SpatialGDK build tool as an Engine plugin.
set BUILD_EXE_PATH="%~dp0..\..\Binaries\ThirdParty\Improbable\Programs\Build.exe"
set SPATIAL_DIR=%UNREAL_PROJECT_DIR%..\spatial\


:Build

if not exist %SPATIAL_DIR% (
	echo Could not find the project's 'spatial' directory! Please ensure your input arguments are correct and your GDK is correctly installed.
	exit /b 1
)

echo Building using project directory: 	%UNREAL_PROJECT_DIR%
echo Building using spatial directory: 	%SPATIAL_DIR%
echo Building using uproject file:		%4

rem First build the spatial worker configs
pushd "%SPATIAL_DIR%"
spatial worker build build-config
popd

rem We pushd to the location of the project file to allow specifying just the .uproject name and not the full path (both are allowed for project plugins).
pushd %UNREAL_PROJECT_DIR%

if not exist %BUILD_EXE_PATH% (
	echo Error: Build executable not found! Please run Setup.bat in your UnrealGDK root to generate it.
	exit /b 1
)

rem Build Unreal project using the SpatialGDK build tool
%BUILD_EXE_PATH% %*
popd
exit /b %ERRORLEVEL%

:MissingParams
echo Missing input arguments! Usage: "<GameName> <Platform> <Configuration> <game.uproject> [-nocompile] <Additional UAT args>"
exit /b 1
