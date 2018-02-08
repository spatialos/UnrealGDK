#!/bin/sh

${UNREAL_HOME}/Engine/Binaries/Win64/UE4Editor.exe \
	${PWD}/"workers\unreal\Game\NUF.uproject" -server -log -workerType UnrealWorker -stdout -nowrite -unattended -nologtimes -nopause -noin -messaging -resX=400 -resY=300 -windowed