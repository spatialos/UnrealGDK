@echo off

setlocal

pushd "%~dp0"

call :MarkStartOfBlock "%~0"

call :MarkStartOfBlock "Setup the git hooks"
    if not exist .git\hooks goto SkipGitHooks

    rem Remove the old post-checkout hook.
    if exist .git\hooks\post-checkout del .git\hooks\post-checkout

    rem Remove the old post-merge hook.
    if exist .git\hooks\post-merge del .git\hooks\post-merge

    rem Add git hook to run Setup.bat when RequireSetup file has been updated.
    echo #!/usr/bin/env bash>.git\hooks\post-merge
    echo changed_files="$(git diff-tree -r --name-only --no-commit-id ORIG_HEAD HEAD)">>.git\hooks\post-merge
    echo check_run() {>>.git\hooks\post-merge
    echo echo "$changed_files" ^| grep --quiet "$1" ^&^& exec $2>>.git\hooks\post-merge
    echo }>>.git\hooks\post-merge
    echo check_run RequireSetup "cmd.exe /c Setup.bat">>.git\hooks\post-merge

    :SkipGitHooks
call :MarkEndOfBlock "Setup the git hooks"

call :MarkStartOfBlock "Check dependencies"

    call "%~dp0SpatialGDK\Build\Scripts\FindMSBuild.bat"

    if not defined MSBUILD_EXE (
        echo Error: Could not find the MSBuild executable. Please make sure you have Microsoft Visual Studio or Microsoft Build Tools installed.
        pause
        exit /b 1
    )

    where spatial >nul
    if ERRORLEVEL 1 (
        echo Error: Could not find spatial. Please make sure you have it installed and the containing directory added to PATH environment variable.
        pause
        exit /b 1
    )
call :MarkEndOfBlock "Check dependencies"

call :MarkStartOfBlock "Setup variables"
    set /p PINNED_CORE_SDK_VERSION=<.\SpatialGDK\Extras\core-sdk.version
    set BUILD_DIR=%~dp0SpatialGDK\Build
    set CORE_SDK_DIR=%BUILD_DIR%\core_sdk
    set WORKER_SDK_DIR=%~dp0SpatialGDK\Source\SpatialGDK\Public\WorkerSDK
    set WORKER_SDK_DIR_OLD=%~dp0SpatialGDK\Source\Public\WorkerSdk
    set BINARIES_DIR=%~dp0SpatialGDK\Binaries\ThirdParty\Improbable

    rem Copy schema to the projects spatial directory.
    set SCHEMA_COPY_DIR=%~dp0..\..\..\spatial\schema\unreal\gdk
    set SCHEMA_STD_COPY_DIR=%~dp0..\..\..\spatial\build\dependencies\schema\standard_library

call :MarkEndOfBlock "Setup variables"

call :MarkStartOfBlock "Clean folders"
    rd /s /q "%CORE_SDK_DIR%"           2>nul
    rd /s /q "%WORKER_SDK_DIR%"         2>nul
    rd /s /q "%WORKER_SDK_DIR_OLD%"     2>nul
    rd /s /q "%BINARIES_DIR%"           2>nul
    rd /s /q "%SCHEMA_STD_COPY_DIR%"    2>nul
call :MarkEndOfBlock "Clean folders"

call :MarkStartOfBlock "Create folders"
    md "%WORKER_SDK_DIR%"            >nul 2>nul
    md "%CORE_SDK_DIR%\schema"       >nul 2>nul
    md "%CORE_SDK_DIR%\tools"        >nul 2>nul
    md "%CORE_SDK_DIR%\worker_sdk"   >nul 2>nul
    md "%BINARIES_DIR%"              >nul 2>nul
    md "%SCHEMA_STD_COPY_DIR%"       >nul 2>nul
call :MarkEndOfBlock "Create folders"

call :MarkStartOfBlock "Retrieve dependencies"
    spatial package retrieve tools           schema_compiler-x86_64-win32           %PINNED_CORE_SDK_VERSION%       "%CORE_SDK_DIR%\tools\schema_compiler-x86_64-win32.zip"
    spatial package retrieve schema          standard_library                       %PINNED_CORE_SDK_VERSION%       "%CORE_SDK_DIR%\schema\standard_library.zip"
    spatial package retrieve worker_sdk      c-dynamic-x86-msvc_md-win32            %PINNED_CORE_SDK_VERSION%       "%CORE_SDK_DIR%\worker_sdk\c-dynamic-x86-msvc_md-win32.zip"
    spatial package retrieve worker_sdk      c-dynamic-x86_64-msvc_md-win32         %PINNED_CORE_SDK_VERSION%       "%CORE_SDK_DIR%\worker_sdk\c-dynamic-x86_64-msvc_md-win32.zip"
    spatial package retrieve worker_sdk      c-dynamic-x86_64-gcc_libstdcpp-linux   %PINNED_CORE_SDK_VERSION%       "%CORE_SDK_DIR%\worker_sdk\c-dynamic-x86_64-gcc_libstdcpp-linux.zip"
call :MarkEndOfBlock "Retrieve dependencies"

call :MarkStartOfBlock "Unpack dependencies"
    powershell -Command "Expand-Archive -Path \"%CORE_SDK_DIR%\worker_sdk\c-dynamic-x86-msvc_md-win32.zip\"             -DestinationPath \"%BINARIES_DIR%\Win32\" -Force; "^
                        "Expand-Archive -Path \"%CORE_SDK_DIR%\worker_sdk\c-dynamic-x86_64-msvc_md-win32.zip\"          -DestinationPath \"%BINARIES_DIR%\Win64\" -Force; "^
                        "Expand-Archive -Path \"%CORE_SDK_DIR%\worker_sdk\c-dynamic-x86_64-gcc_libstdcpp-linux.zip\"    -DestinationPath \"%BINARIES_DIR%\Linux\" -Force; "^
                        "Expand-Archive -Path \"%CORE_SDK_DIR%\tools\schema_compiler-x86_64-win32.zip\"                 -DestinationPath \"%BINARIES_DIR%\Programs\" -Force; "^
                        "Expand-Archive -Path \"%CORE_SDK_DIR%\schema\standard_library.zip\"                            -DestinationPath \"%BINARIES_DIR%\Programs\schema\" -Force;"

    xcopy /s /i /q "%BINARIES_DIR%\Win64\include" "%WORKER_SDK_DIR%"
call :MarkEndOfBlock "Unpack dependencies"

call :MarkStartOfBlock "Copy standard library schema"
    echo Copying standard library schemas to "%SCHEMA_STD_COPY_DIR%"
    xcopy /s /i /q "%BINARIES_DIR%\Programs\schema" "%SCHEMA_STD_COPY_DIR%"
call :MarkEndOfBlock "Copy standard library schema"

call :MarkStartOfBlock "Copy GDK schema"
    rd /s /q "%SCHEMA_COPY_DIR%"      2>nul
    md "%SCHEMA_COPY_DIR%"       >nul 2>nul

    echo Copying schemas to "%SCHEMA_COPY_DIR%".
    xcopy /s /i /q "%~dp0\SpatialGDK\Extras\schema" "%SCHEMA_COPY_DIR%"
call :MarkEndOfBlock "Copy GDK schema"

call :MarkStartOfBlock "Build C# utilities"
    %MSBUILD_EXE% /nologo /verbosity:minimal .\SpatialGDK\Build\Programs\Improbable.Unreal.Scripts\Improbable.Unreal.Scripts.sln /property:Configuration=Release
call :MarkEndOfBlock "Build C# utilities"

call :MarkEndOfBlock "%~0"

popd

echo UnrealGDK build completed successfully^!

if not defined NO_PAUSE (
    pause
)

exit /b %ERRORLEVEL%

:MarkStartOfBlock
echo Starting: %~1
exit /b 0

:MarkEndOfBlock
echo Finished: %~1
exit /b 0
