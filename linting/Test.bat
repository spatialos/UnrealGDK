@if not defined TEAMCITY_CAPTURE_ENV ( echo off ) else ( echo on )

setlocal

pushd "%~dp0..\"

call :MarkStartOfBlock "%~0"

set LINTER_EXE="Binaries\ThirdParty\Improbable\Programs\Linter.exe"
if not exist %LINTER_EXE% (
    echo Error: Could not find %LINTER_EXE%. Please run Build.bat to generate it.
    if not defined TEAMCITY_CAPTURE_ENV pause
    exit /b 1
)

%LINTER_EXE% check "Source\SpatialGDK\Public" ^
                   "Source\SpatialGDK\Private" ^
                   "Source\SpatialGDK\Legacy" ^
                   "Plugins"

call :MarkEndOfBlock "%~0"

popd

if not defined TEAMCITY_CAPTURE_ENV pause
exit /b %ERRORLEVEL%

:MarkStartOfBlock
if defined TEAMCITY_CAPTURE_ENV (
    echo ##teamcity[blockOpened name='%~1']
) else (
    echo Starting: %~1
)
exit /b 0

:MarkEndOfBlock
if defined TEAMCITY_CAPTURE_ENV (
    echo ##teamcity[blockClosed name='%~1']
) else (
    echo Finished: %~1
)
exit /b 0
