@echo off

IF NOT "%~3"=="" IF "%~4"=="" GOTO START
ECHO This script requires three parameters (the first parameter is the project root and the other parameters should be defined relative to project root):
ECHO - path to your project root containing the 'spatial' folder
ECHO - path to external schema directory
ECHO - target output folder for generated code
exit /b 1

:START

call :MarkStartOfBlock "Setup variables"
  pushd %1
    set GAME_FOLDER=%cd%
  popd
  pushd %~dp0\..\..\..
    set GDK_FOLDER=%cd%
    set SCHEMA_COMPILER_PATH=%GDK_FOLDER%\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\schema_compiler.exe
    set CODEGEN_EXE_PATH=%GDK_FOLDER%\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\CodeGenerator.exe
    set SCHEMA_STD_COPY_DIR=%GAME_FOLDER%\spatial\build\dependencies\schema\standard_library
    set SPATIAL_SCHEMA_FOLDER=%GAME_FOLDER%\spatial\schema
    set BUNDLE_CACHE_DIR=%GDK_FOLDER%\SpatialGDK\Intermediate\ExternalSchemaCodegen
    set SCHEMA_BUNDLE_FILE_NAME=external_schema_bundle.json
  popd
call :MarkEndOfBlock "Setup variables"

call :MarkStartOfBlock "Clean folders"
    rd /s /q "%BUNDLE_CACHE_DIR%"           2>nul
call :MarkEndOfBlock "Clean folders"

call :MarkStartOfBlock "Create folders"
    md "%BUNDLE_CACHE_DIR%"            >nul 2>nul
call :MarkEndOfBlock "Create folders"


if not exist "%SCHEMA_COMPILER_PATH%" (
    echo Error: Schema compiler executable not found at "%SCHEMA_COMPILER_PATH%" ! Please run Setup.bat in your UnrealGDK root to generate it.
    exit /b 1
)

if not exist "%SCHEMA_STD_COPY_DIR%" (
    echo Error: Could not locate SpatialOS standard library files at "%SCHEMA_STD_COPY_DIR%" ! Please run Setup.bat in your UnrealGDK root to generate it.
    exit /b 1
)

call :MarkStartOfBlock "Collecting external schema files"
set EXTERNAL_SCHEMA_FILES=
setlocal enabledelayedexpansion
FOR /F %%i in ('dir /s/b "%GAME_FOLDER%\%2\*.schema"') do ( set "EXTERNAL_SCHEMA_FILES=!EXTERNAL_SCHEMA_FILES! %%i" )
setlocal disabledelayedexpansion
call :MarkEndOfBlock "Collecting external schema files"

call :MarkStartOfBlock "Running schema compiler"
%SCHEMA_COMPILER_PATH% --schema_path="%SPATIAL_SCHEMA_FOLDER%" --bundle_json_out="%BUNDLE_CACHE_DIR%\%SCHEMA_BUNDLE_FILE_NAME%" %EXTERNAL_SCHEMA_FILES% || exit /b 1
call :MarkEndOfBlock "Running schema compiler"

if not exist "%CODEGEN_EXE_PATH%" (
    echo Error: Codegen executable not found at "%CODEGEN_EXE_PATH%"! Please run Setup.bat in your UnrealGDK root to generate it.
    exit /b 1
)

call :MarkStartOfBlock "Running code generator"
%CODEGEN_EXE_PATH% --input-bundle "%BUNDLE_CACHE_DIR%\%SCHEMA_BUNDLE_FILE_NAME%" --output-dir "%GAME_FOLDER%\%3"
if ERRORLEVEL 1 (
    echo Error: Code generation failed
    pause
    exit /b 1
)
echo Code successfully generated at "%GAME_FOLDER%\%2"
call :MarkEndOfBlock "Running code generator"

exit /b %ERRORLEVEL%

:MarkStartOfBlock
echo Starting: %~1
exit /b 0

:MarkEndOfBlock
echo Finished: %~1
exit /b 0
