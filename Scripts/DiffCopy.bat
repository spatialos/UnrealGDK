@echo off

cd "%~dp0..\"

if not exist "Game\Intermediate\Improbable" mkdir "Game\Intermediate\Improbable"

csc "Scripts/DiffCopy.cs" /nologo /out:"Game\Intermediate\Improbable\DiffCopy.exe" || exit /b 1

Game\Intermediate\Improbable\DiffCopy.exe %*

exit /b %ERRORLEVEL%
