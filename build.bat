@echo off

setlocal

pushd "%~dp0"

rem Start Section: Check dependencies.
    if not defined UNREAL_HOME (
        echo Error: Please set UNREAL_HOME environment variable to point to the Unreal Engine folder.
        pause
        exit /b 1
    )

    rem Use Unreal Engine's script to get the path to MSBuild.
    call "%UNREAL_HOME%\Engine\Build\BatchFiles\GetMSBuildPath.bat"

    if not defined MSBUILD_EXE (
        echo Error: Could not find the MSBuild executable. Please make sure you have Microsoft Visual Studio or Microsoft Build Tools installed.
        pause
        exit /b 1
    )

    if defined CSC_EXE goto csc_defined

    rem Get directory from MSBuild path.
    for /f "delims=" %%i in (%MSBUILD_EXE%) do (
        set MSBUILD_DIR=%%~dpi
    )

    rem C# compiler should live in the same directory or inside Roslyn subfolder.
    if exist "%MSBUILD_DIR%csc.exe" (
        set CSC_EXE="%MSBUILD_DIR%csc.exe"
    )
    if exist "%MSBUILD_DIR%Roslyn\csc.exe" (
        set CSC_EXE="%MSBUILD_DIR%Roslyn\csc.exe"
    )
    if not defined CSC_EXE (
        echo Error: Could not find csc.exe. If you have C# compiler installed in a different location, please specify it in CSC_EXE environment variable.
        pause
        exit /b 1
    )
    echo Found csc.exe: %CSC_EXE%

    :csc_defined

    where spatial >nul
    if errorlevel 1 (
        echo Error: Could not find spatial. Please make sure you have it installed and the containing directory added to PATH environment variable.
        pause
        exit /b 1
    )
rem End Section: Check dependencies.

rem Start Section: Setup variables.
    set /p PINNED_CORE_SDK_VERSION=<core-sdk.version
    set /p PINNED_CODE_GENERATOR_VERSION=<code-generator.version

    set BUILD_DIR=%~dp0build
    set CORE_SDK_DIR=%BUILD_DIR%\core_sdk

    set PACKAGE_TARGET_DIR=%~dp0packages
    set CACHE_PATH=%HOMEDRIVE%%HOMEPATH%\.imp_nuget

    set WORKER_SDK_DIR=%~dp0Source\SpatialGDK\Public\WorkerSdk
    set BINARIES_DIR=%~dp0Binaries\ThirdParty\Improbable
rem End Section: Setup variables.

rem Start Section: Clean folders.
    rd /s /q %BUILD_DIR%          2>nul
    rd /s /q %PACKAGE_TARGET_DIR% 2>nul
    rd /s /q %WORKER_SDK_DIR%     2>nul
    rd /s /q %BINARIES_DIR%       2>nul
rem End Section: Clean folders.

rem Start Section: Create folders.
    md %PACKAGE_TARGET_DIR%        >nul 2>nul
    md %WORKER_SDK_DIR%            >nul 2>nul
    md %CORE_SDK_DIR%\schema       >nul 2>nul
    md %CORE_SDK_DIR%\tools        >nul 2>nul
    md %CORE_SDK_DIR%\worker_sdk   >nul 2>nul
    md %BUILD_DIR%\code_generation >nul 2>nul
    md %BINARIES_DIR%              >nul 2>nul
rem End Section: Create folders.

rem Start Section: Retrieve dependencies.
    spatial package retrieve tools           schema_compiler-x86_64-win32     %PINNED_CORE_SDK_VERSION%       %CORE_SDK_DIR%\tools\schema_compiler-x86_64-win32.zip
    spatial package retrieve schema          standard_library                 %PINNED_CORE_SDK_VERSION%       %CORE_SDK_DIR%\schema\standard_library.zip
    spatial package retrieve worker_sdk      core-dynamic-x86-win32           %PINNED_CORE_SDK_VERSION%       %CORE_SDK_DIR%\worker_sdk\core-dynamic-x86-win32.zip
    spatial package retrieve worker_sdk      core-dynamic-x86_64-win32        %PINNED_CORE_SDK_VERSION%       %CORE_SDK_DIR%\worker_sdk\core-dynamic-x86_64-win32.zip
    spatial package retrieve worker_sdk      core-dynamic-x86_64-linux        %PINNED_CORE_SDK_VERSION%       %CORE_SDK_DIR%\worker_sdk\core-dynamic-x86_64-linux.zip
    spatial package retrieve code_generation Improbable.CodeGeneration        %PINNED_CODE_GENERATOR_VERSION% %BUILD_DIR%\code_generation\Improbable.CodeGeneration.zip
    rem Download the C++ SDK for its headers, only.
    spatial package retrieve worker_sdk      cpp-static-x86_64-msvc_mtd-win32 %PINNED_CORE_SDK_VERSION%       %CORE_SDK_DIR%\cpp-static-x86_64-msvc_mtd-win32.zip
rem End Section: Retrieve dependencies.

rem Start Section: Unpack dependencies.
    powershell -Command "Expand-Archive -Path %CORE_SDK_DIR%\cpp-static-x86_64-msvc_mtd-win32.zip       -DestinationPath %CORE_SDK_DIR%\cpp-src -Force; "^
                        "Expand-Archive -Path %CORE_SDK_DIR%\worker_sdk\core-dynamic-x86-win32.zip      -DestinationPath %BINARIES_DIR%\Win32 -Force; "^
                        "Expand-Archive -Path %CORE_SDK_DIR%\worker_sdk\core-dynamic-x86_64-win32.zip   -DestinationPath %BINARIES_DIR%\Win64 -Force; "^
                        "Expand-Archive -Path %CORE_SDK_DIR%\worker_sdk\core-dynamic-x86_64-linux.zip   -DestinationPath %BINARIES_DIR%\Linux -Force; "^
                        "Expand-Archive -Path %CORE_SDK_DIR%\tools\schema_compiler-x86_64-win32.zip     -DestinationPath %BINARIES_DIR%\Programs -Force; "^
                        "Expand-Archive -Path %CORE_SDK_DIR%\schema\standard_library.zip                -DestinationPath %BINARIES_DIR%\Programs\schema -Force; "^
                        "Expand-Archive -Path %BUILD_DIR%\code_generation\Improbable.CodeGeneration.zip -DestinationPath %PACKAGE_TARGET_DIR%\Improbable.CodeGeneration -Force; "^
                        "Expand-Archive -Path %~dp0Source\Programs\Improbable.Unreal.CodeGeneration.Test\NUnit\NUnit.zip -DestinationPath %PACKAGE_TARGET_DIR% -Force"

    rem Include the WorkerSDK header files.
    xcopy /s /i /q %CORE_SDK_DIR%\cpp-src\include %WORKER_SDK_DIR%
rem End Section: Unpack dependencies.

rem Start Section: Build CodeGeneration.
    %MSBUILD_EXE% /nologo /verbosity:minimal Source\Programs\Improbable.Unreal.CodeGeneration\UnrealCodeGeneration.sln /property:Configuration=Release /property:SolutionDir=..\

    xcopy /i /q Source\Programs\Improbable.Unreal.CodeGeneration\bin\Release\*.dll %BINARIES_DIR%\Programs
    xcopy /i /q Source\Programs\Improbable.Unreal.CodeGeneration\bin\Release\*.exe %BINARIES_DIR%\Programs
rem End Section: Build CodeGeneration.

rem Start Section: Build C# utilities.
    %CSC_EXE% Scripts\Build.cs   Scripts\Codegen.cs Scripts\Common.cs -main:Improbable.Build -nologo -out:%BINARIES_DIR%\Programs\Build.exe
    %CSC_EXE% Scripts\Codegen.cs Scripts\Common.cs                                           -nologo -out:%BINARIES_DIR%\Programs\Codegen.exe
    %CSC_EXE% Scripts\DiffCopy.cs                                                            -nologo -out:%BINARIES_DIR%\Programs\DiffCopy.exe
rem End Section: Build C# utilities.

popd

echo UnrealGDK build completed successfully^!
pause
exit /b %ERRORLEVEL%