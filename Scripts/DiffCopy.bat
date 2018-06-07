@echo off

pushd "%~dp0..\"

if not exist "Intermediate\Improbable" mkdir "Intermediate\Improbable"

csc "Scripts/DiffCopy.cs" /nologo /out:"Intermediate\Improbable\DiffCopy.exe" || exit /b 1

Intermediate\Improbable\DiffCopy.exe %*

popd

exit /b %ERRORLEVEL%
