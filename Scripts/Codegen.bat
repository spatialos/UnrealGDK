@echo off

cd "%~dp0..\..\"

:: Back compat: ensure that the standard schema is available for the `spatial upload` command.
:: It's distributed with the CodeGenerator, so it's copied from there into the expected location.

set SPATIAL_DEPENDENCY_DIR="spatial\build\dependencies\schema\standard_library"
if exist "%SPATIAL_DEPENDENCY_DIR%" rd /S /Q "%SPATIAL_DEPENDENCY_DIR%"
if not exist "%SPATIAL_DEPENDENCY_DIR%" mkdir "%SPATIAL_DEPENDENCY_DIR%"
echo Installing standard library schema.
xcopy /S /Y /Q "Game\Binaries\ThirdParty\Improbable\Programs\schema" "%SPATIAL_DEPENDENCY_DIR%"

if not exist "Game\Intermediate\Improbable" mkdir "Game\Intermediate\Improbable"

csc "Game/Scripts/Codegen.cs" "Game/Scripts/Common.cs" /nologo /out:"Game\Intermediate\Improbable\Codegen.exe" || exit /b 1

Game\Intermediate\Improbable\Codegen.exe %*

exit /b %ERRORLEVEL%
