@echo off

rem Usage: <GameName> <Platform> <Configuration> <game.uproject> [-nocompile] <Additional UAT args>

rem If we are running as a project plugin then this will be the path to the spatial directory.
:BuildAsProjectPlugin
if exist "%~dp0..\..\..\..\..\..\spatial" (
	echo Building as project plugin
	set SpatialDir="%~dp0..\..\..\..\..\..\spatial"
	set UnrealProjectDir="%~dp0..\..\..\..\..\"
	set BUILD_EXE_PATH="Plugins\UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\Build.exe"
	goto Build
)


:BuildAsEnginePlugin
rem If we are running as an Engine plugin then we need a full path to the .uproject file!

rem Grab the project path from the .uproject file.
set uproject=%4

if not exist %uproject% (
	echo To use BuildWorker.bat with an Engine plugin installation you must provide a full path to your .project file.
	exit /b 1
)

for %%i in (%uproject%) do (
	rem file drive + file directory
	set UnrealProjectDir="%%~di%%~pi"
)

set SpatialDir=%UnrealProjectDir%..\spatial\

if not exist %SpatialDir% (
	echo Could not find the projects 'spatial' directory! Please ensure your input arguments are correct and your GDK is correctly installed.
	exit /b 1
)

rem Path to the SpatialGDK build tool as an Engine plugin.
set BUILD_EXE_PATH="%~dp0..\..\Binaries\ThirdParty\Improbable\Programs\Build.exe"


:Build
echo Building using project directory: 	%UnrealProjectDir%
echo Building using spatial directory: 	%SpatialDir%
echo Building using project file:		%uproject%

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
