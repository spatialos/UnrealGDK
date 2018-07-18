@echo off

REM Use Unreal tooling to resolve MSBuild path
call %UNREAL_HOME%\Engine\Build\BatchFiles\GetMSBuildPath.bat

echo %MSBUILD_EXE%

exit /b 0
