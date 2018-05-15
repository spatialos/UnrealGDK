@echo off

cd "%~dp0..\..\"

if not exist "Game\Intermediate\Improbable" mkdir "Game\Intermediate\Improbable"

csc "Game/Scripts/Build.cs" "Game/Scripts/Codegen.cs" "Game/Scripts/Common.cs" /main:"Improbable.Build" /nologo /out:"Game\Intermediate\Improbable\Build.exe" || exit /b 1

Game\Intermediate\Improbable\Build.exe %*

exit /b %ERRORLEVEL%
