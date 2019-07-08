@echo off

rem Usage: <GameName> <Platform> <Configuration> <game.uproject> [-nocompile] <Additional UAT args>

rem Try and build as a project plugin first, check for a project plugin structure.
set UnrealProjectDir="%~dp0..\..\..\..\..\"

for /f "delims=" %%A in (' powershell -Command "(Get-ChildItem -Path "%UnrealProjectDir%" *.uproject).FullName" ') do set FoundUproject="%%A"

if "%FoundUproject%"=="" (
	goto :BuildAsEnginePlugin
)

rem If we are running as a project plugin then this will be the path to the spatial directory.
echo Building as project plugin
set SpatialDir="%~dp0..\..\..\..\..\..\spatial"
set BUILD_EXE_PATH="Plugins\UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\Build.exe"
goto :Build


:BuildAsEnginePlugin
rem If we are running as an Engine plugin then we need a full path to the .uproject file!
set UPROJECT=%4

if not exist %UPROJECT% (
	echo To use BuildWorker.bat with an Engine plugin installation you must provide a full path to your .uproject file.
	exit /b 1
)

rem Grab the project path from the .uproject file.
for %%i in (%UPROJECT%) do (
	rem file drive + file directory
	set UnrealProjectDir="%%~di%%~pi"
)

rem Path to the SpatialGDK build tool as an Engine plugin.
set BUILD_EXE_PATH="%~dp0..\..\Binaries\ThirdParty\Improbable\Programs\Build.exe"
set SpatialDir=%UnrealProjectDir%..\spatial\


:Build

if not exist %SpatialDir% (
	echo Could not find the projects 'spatial' directory! Please ensure your input arguments are correct and your GDK is correctly installed.
	exit /b 1
)

echo Building using project directory: 	%UnrealProjectDir%
echo Building using spatial directory: 	%SpatialDir%
echo Building using uproject file:		%4

rem First build the spatial worker configs
pushd "%SpatialDir%"
spatial worker build build-config
popd

rem We pushd to the location of the project file to allow specifying just the .uproject name and not the full path (both are allowed for project plugins).
pushd %UnrealProjectDir%

if not exist %BUILD_EXE_PATH% (
	echo Error: Build executable not found! Please run Setup.bat in your UnrealGDK root to generate it.
	exit /b 1
)

rem Build Unreal project using the SpatialGDK build tool
%BUILD_EXE_PATH% %*
popd
exit /b %ERRORLEVEL%
