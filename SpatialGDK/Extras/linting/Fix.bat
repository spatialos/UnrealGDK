@echo off

setlocal

pushd "%~dp0..\"

set LINTER_EXE="Binaries\ThirdParty\Improbable\Programs\Linter.exe"
if not exist %LINTER_EXE% (
    echo Error: Could not find %LINTER_EXE%. Please run Setup.bat in your UnrealGDK root to generate it.
    pause
    exit /b 1
)

echo This will analyze the SpatialGDK module and plugins for any formatting issues and fix them.
set /p CONTINUE=Would you like to continue?[Y/N]:
if /I "%CONTINUE%" == "Y" (
    %LINTER_EXE% fix "Source\SpatialGDK\Public" ^
                     "Source\SpatialGDK\Private" ^
                     "Source\SpatialGDK\Legacy" ^
                     "Plugins"
)

popd

pause
exit /b %ERRORLEVEL%
