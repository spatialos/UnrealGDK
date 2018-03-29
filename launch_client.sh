#!/bin/sh

${UNREAL_HOME}/Engine/Binaries/Win64/UE4Editor.exe \
	${PWD}/"workers\unreal\Game\SampleGame.uproject" -game -log -workerType UnrealClient -stdout -nowrite -unattended -nologtimes -nopause -noin -messaging -windowed -ResX=800 -ResY=600