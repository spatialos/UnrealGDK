@echo off

pushd "%1"

for /R %%x in (*.cc) do ren "%%x" *.cpp

popd

exit /b %ERRORLEVEL%
