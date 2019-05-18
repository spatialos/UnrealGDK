@echo off

IF NOT "%~2"=="" IF "%~3"=="" GOTO START
ECHO This script requires two parameters:
ECHO - path to external schema directory
ECHO - target output folder for generated code
exit /b 1

:START

call :MarkStartOfBlock "Setup variables"
    set GDK_FOLDER=%~dp0\..\..\..
    set GAME_FOLDER=%GDK_FOLDER%\..\..\..
    set SCHEMA_COMPILER_PATH=%GDK_FOLDER%\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\schema_compiler.exe
    set CODEGEN_EXE_PATH=%GDK_FOLDER%\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\CodeGenerator.exe
    set SCHEMA_STD_COPY_DIR=%GAME_FOLDER%\spatial\build\dependencies\schema\standard_library
	set BUNDLE_CACHE_DIR=%GDK_FOLDER%\SpatialGDK\Intermediate\ExternalSchemaCodegen
	set SCHEMA_BUNDLE_FILE_NAME=external_schema_bundle.json
call :MarkEndOfBlock "Setup variables"

call :MarkStartOfBlock "Clean folders"
    rd /s /q "%BUNDLE_CACHE_DIR%"           2>nul
call :MarkEndOfBlock "Clean folders"

call :MarkStartOfBlock "Create folders"
    md "%BUNDLE_CACHE_DIR%"            >nul 2>nul
call :MarkEndOfBlock "Create folders"


if not exist %SCHEMA_COMPILER_PATH% (
	echo Error: Schema compiler executable not found at %SCHEMA_COMPILER_PATH% ! Please run Setup.bat in your UnrealGDK root to generate it.
	exit /b 1
)

if not exist %SCHEMA_STD_COPY_DIR% (
	echo Error: Could not locate SpatialOS standard library files at %SCHEMA_STD_COPY_DIR% ! Please run Setup.bat in your UnrealGDK root to generate it.
	exit /b 1
)

call :MarkStartOfBlock "Running schema compiler"
REM %SCHEMA_COMPILER_PATH% --schema_path=%SCHEMA_STD_COPY_DIR%  --schema_path=%1 --bundle_json_out=%BUNDLE_CACHE_DIR%\%SCHEMA_BUNDLE_FILE_NAME% --load_all_schema_on_schema_path || exit /b 1
%SCHEMA_COMPILER_PATH% --schema_path=%1 --bundle_json_out=%BUNDLE_CACHE_DIR%\%SCHEMA_BUNDLE_FILE_NAME% --load_all_schema_on_schema_path || exit /b 1
call :MarkEndOfBlock "Running schema compiler"

if not exist %CODEGEN_EXE_PATH% (
	echo Error: Codegen executable not found at %CODEGEN_EXE_PATH%! Please run Setup.bat in your UnrealGDK root to generate it.
	exit /b 1
)

call :MarkStartOfBlock "Running code generator"
%CODEGEN_EXE_PATH% --input-bundle %BUNDLE_CACHE_DIR%\%SCHEMA_BUNDLE_FILE_NAME% --output-dir %2
echo Code successfully generated at %2
call :MarkEndOfBlock "Running code generator"

exit /b %ERRORLEVEL%

:MarkStartOfBlock
echo Starting: %~1
exit /b 0

:MarkEndOfBlock
echo Finished: %~1
exit /b 0
