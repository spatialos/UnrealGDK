#!/usr/bin/env bash

set -e -u -x -o pipefail

cd "$(dirname "$0")/../"

source ci/pinned-tools.sh
source ci/profiling.sh

if ! isWindows ; then
  echo "Unreal GDK can only be built on Windows."
  exit 0
fi

markStartOfBlock "$0"

markStartOfBlock "Setup variables"

  pushd "SpatialGDK/Extras"

    UNREAL_VERSION=$(cat unreal-engine.version)
    UNREAL_HOME=C:/Unreal/UnrealEngine-${UNREAL_VERSION}

  popd

markEndOfBlock "Setup variables"

markStartOfBlock "Setup dependencies"

  ./Setup.bat

markEndOfBlock "Setup dependencies"

markStartOfBlock "Build the GDK (Windows)"

  pushd "SpatialGDK"

  "${UNREAL_HOME}/Engine/Build/BatchFiles/RunUAT.bat" BuildPlugin -Plugin="$PWD/SpatialGDK.uplugin" -TargetPlatforms=Win64 -Package="$PWD/Intermediate/BuildPackage/Win64"

  popd

markEndOfBlock "Build the GDK (Windows)"

markEndOfBlock "$0"
