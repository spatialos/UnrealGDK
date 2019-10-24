rem Convenient code to find MSBuild.exe from https://github.com/microsoft/vswhere 
rem We only support VS2017 so only look for MS_Build 15.0 

@echo off

set MSBUILD_EXE=

if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
	for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere" -prerelease -latest -requires Microsoft.Component.MSBuild -property installationPath`) do (
		if exist "%%i\MSBuild\15.0\Bin\MSBuild.exe" (
			set MSBUILD_EXE="%%i\MSBuild\15.0\Bin\MSBuild.exe"
			exit /b 0
		)
	)
)

rem As a backup, if we couldn't find MSBuild via vswhere then ask the registry.
for /f "usebackq tokens=2,*" %%A in (`reg query "HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\VisualStudio\SxS\VS7" /v 15.0`) do (
	if exist "%%BMSBuild\15.0\Bin\MSBuild.exe" (
		set MSBUILD_EXE="%%BMSBuild\15.0\Bin\MSBuild.exe"
		exit /b 0
	)
)

exit /b 1
