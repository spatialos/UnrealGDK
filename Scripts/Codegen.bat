@echo off

pushd "%~dp0..\"

if not exist "Intermediate\Improbable" mkdir "Intermediate\Improbable"

csc "Scripts/Codegen.cs" "Scripts/Common.cs" /nologo /out:"Intermediate\Improbable\Codegen.exe" || exit /b 1

Intermediate\Improbable\Codegen.exe %*

exit /b %ERRORLEVEL%
