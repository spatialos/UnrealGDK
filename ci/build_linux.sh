#!/usr/bin/env bash

set -e -u -x -o pipefail

cd "$(dirname "$0")/../"

source ci/profiling.sh

markStartOfBlock "$0"

markStartOfBlock "Setup dependencies"

   # ./Setup.bat

markEndOfBlock "Setup dependencies"

markStartOfBlock "Build the TestSuite (Linux)"

    # pushd "SpatialGDK"

    # "${UNREAL_HOME}/Engine/Build/BatchFiles/RunUAT.bat" BuildPlugin -Plugin="$PWD/SpatialGDK.uplugin" -TargetPlatforms=Linux -Package="$PWD/Intermediate/BuildPackage/Linux"

    # popd

markEndOfBlock "Build the TestSuite (Linux)"

markEndOfBlock "$0"
