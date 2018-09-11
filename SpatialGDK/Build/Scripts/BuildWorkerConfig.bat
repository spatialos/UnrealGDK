@echo off

pushd "%~dp0..\..\..\..\..\spatial"
spatial worker build build-config
popd
