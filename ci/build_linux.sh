#!/usr/bin/env bash

set -e -u -x -o pipefail

cd "$(dirname "$0")/../"

source ci/pinned-tools.sh

if ! isTeamCity ; then
  echo "This script should only be run on the CI agents."
  exit 0
fi

if ! isWindows ; then
  echo "TestSuite can only be built on Windows."
  exit 0
fi

source ci/profiling.sh

markStartOfBlock "$0"

markStartOfBlock "Setup dependencies"

  ./Setup.bat

markEndOfBlock "Setup dependencies"

markStartOfBlock "Build the TestSuite (Linux)"

  pushd "SpatialGDK"

  "${UNREAL_HOME}/Engine/Build/BatchFiles/RunUAT.bat" BuildPlugin -Plugin="$PWD/SpatialGDK.uplugin" -TargetPlatforms=Linux -Package="$PWD/Intermediate/BuildPackage/Linux"

  popd

markEndOfBlock "Build the TestSuite (Linux)"

markEndOfBlock "$0"
