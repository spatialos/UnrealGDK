@echo off

cd "%~dp0..\..\"

if not exist "Game\Intermediate\Improbable" mkdir "Game\Intermediate\Improbable"

csc "Game/Scripts/Codegen.cs" "Game/Scripts/Common.cs" /nologo /out:"Game\Intermediate\Improbable\Codegen.exe" || exit /b 1

Game\Intermediate\Improbable\Codegen.exe %*

exit /b %ERRORLEVEL%
