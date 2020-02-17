rem Convenient code to find MSBuild.exe from https://github.com/microsoft/vswhere

@echo off

if not defined MSBUILD_EXE (
	set MSBUILD_EXE=

if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
	for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
		if exist %%i (
			set MSBUILD_EXE="%%i"
			exit /b 0
		)
	)
)

	for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere" -prerelease -latest -requires Microsoft.Component.MSBuild -property installationPath`) do (
		if exist "%%i\MSBuild\Current\Bin\MSBuild.exe" (
			set MSBUILD_EXE="%%i\MSBuild\Current\Bin\MSBuild.exe"
			exit /b 0
		)

		if exist "%%i\MSBuild\15.0\Bin\MSBuild.exe" (
			set MSBUILD_EXE="%%i\MSBuild\15.0\Bin\MSBuild.exe"
			exit /b 0
		)
	)

	exit /b 1
)
