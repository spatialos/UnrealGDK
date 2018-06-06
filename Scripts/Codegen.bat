@echo off

cd "%~dp0..\"

:: Back compat: ensure that the standard schema is available for the `spatial upload` command.
:: It's distributed with the CodeGenerator, so it's copied from there into the expected location.

set SPATIAL_DEPENDENCY_DIR="..\spatial\build\dependencies\schema\standard_library"
if exist "%SPATIAL_DEPENDENCY_DIR%" rd /S /Q "%SPATIAL_DEPENDENCY_DIR%"
if not exist "%SPATIAL_DEPENDENCY_DIR%" mkdir "%SPATIAL_DEPENDENCY_DIR%"
echo Installing standard library schema.
xcopy /S /Y /Q "Binaries\ThirdParty\Improbable\Programs\schema" "%SPATIAL_DEPENDENCY_DIR%"

if not exist "Intermediate\Improbable" mkdir "Intermediate\Improbable"

csc "Scripts/Codegen.cs" "Scripts/Common.cs" /nologo /out:"Intermediate\Improbable\Codegen.exe" || exit /b 1

Intermediate\Improbable\Codegen.exe %*

exit /b %ERRORLEVEL%
