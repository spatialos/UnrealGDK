@echo off

cd "%~dp0..\..\"

pushd spatial
spatial worker build build-config
popd
