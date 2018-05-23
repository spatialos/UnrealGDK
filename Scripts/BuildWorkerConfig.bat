@echo off

cd "%~dp0..\..\"

cd spatial
spatial worker build build-config
