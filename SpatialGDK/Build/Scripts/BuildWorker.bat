@echo off

rem Input args: BuildName, Platform, BuildConfiguration, uproject 

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
rem If we are running as an Engine plugin then find the project path and spatial path.

rem Grab the project path from the .uproject file.
set uproject=%4
for %%i in (%uproject%) do (
	rem file drive + file directory
	set ProjectDir=%%~di%%~pi
)

set SpatialDir=%ProjectDir%spatial\
set UnrealProjectDir=%ProjectDir%%PROJECT_PATH%\

if not exist %SpatialDir% (
	goto: SpatialNotExist
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

rem We pushd to the location of the project file to allow specifying just the .uproject name and not the full path (both are allowed).
pushd %UnrealProjectDir%

if not exist %BUILD_EXE_PATH% (
	echo Error: Build executable not found! Please run Setup.bat in your UnrealGDK root to generate it.
	exit /b 1
)

rem Build Unreal project using the SpatialGDK build tool
%BUILD_EXE_PATH% %*
popd
exit /b %ERRORLEVEL%


:SpatialNotExist
echo Could not find the projects 'spatial' directory! Please ensure your input arguments are correct and your GDK is correctly installed.
exit /b 1
