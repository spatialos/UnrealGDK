@echo off

pushd "%~dp0..\"

call "Scripts\BuildWorkerConfig.bat"

if not exist "Intermediate\Improbable" mkdir "Intermediate\Improbable"

csc "Scripts/Build.cs" "Scripts/Codegen.cs" "Scripts/Common.cs" /main:"Improbable.Build" /nologo /out:"Intermediate\Improbable\Build.exe" || exit /b 1

Intermediate\Improbable\Build.exe %*

popd

exit /b %ERRORLEVEL%
