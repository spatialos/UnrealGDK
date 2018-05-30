@echo off

cd "%~dp0..\..\"

:: Back compat: ensure that the standard schema is available for the `spatial upload` command.
:: It's distributed with the CodeGenerator, so it's copied from there into the expected location.

set SPATIAL_DEPENDENCY_DIR="spatial\build\dependencies\schema\standard_library"
if exist "%SPATIAL_DEPENDENCY_DIR%" rd /S /Q "%SPATIAL_DEPENDENCY_DIR%"
if not exist "%SPATIAL_DEPENDENCY_DIR%" mkdir "%SPATIAL_DEPENDENCY_DIR%"
echo Installing standard library schema.
:: IMPROBABLE: giray changing hardcoded Game folder to Scavengers
xcopy /S /Y /Q "Scavengers\Binaries\ThirdParty\Improbable\Programs\schema" "%SPATIAL_DEPENDENCY_DIR%"

if not exist "Scavengers\Intermediate\Improbable" mkdir "Scavengers\Intermediate\Improbable"

csc "Scavengers/Scripts/Codegen.cs" "Scavengers/Scripts/Common.cs" /nologo /out:"Scavengers\Intermediate\Improbable\Codegen.exe" || exit /b 1

Scavengers\Intermediate\Improbable\Codegen.exe %*

exit /b %ERRORLEVEL%