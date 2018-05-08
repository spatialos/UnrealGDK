#!/usr/bin/env bash

set -e -u -o pipefail

cd "$(dirname "$0")"

source ci/pinned-tools.sh
source ci/profiling.sh

if ! isWindows ; then
  echo "Unreal GDK is only supported on Windows."
  exit 0
fi

if [ -z "${1+x}" ]; then
  echo "No game install path was provided, please provide the path to the game that you would like to install the SpatialOS Unreal GDK to."
  exit 1
fi

INSTALL_PATH="$1"
BUILT_PLUGINSPATH="Plugins\SpatialGDK"
BUILT_MODULEPATH="Source\SpatialGDK"
BUILT_SCRIPTSPATH="Scripts"
BUILT_BINARIESPATH="Binaries"

TARGET_PLUGINSPATH="${INSTALL_PATH}\workers\unreal\Game\Plugins"
TARGET_MODULEPATH="${INSTALL_PATH}\workers\unreal\Game\Source"
TARGET_SCRIPTSPATH="${INSTALL_PATH}\workers\unreal\Game"
TARGET_BINARIESPATH="${INSTALL_PATH}\workers\unreal\Game"

markStartOfBlock "$0"

markStartOfBlock "Build the SpatialOS Unreal GDK"

ci/build.sh

markEndOfBlock "Build the SpatialOS Unreal GDK"

markStartOfBlock "Ensure directories exist"

mkdir -p "${TARGET_PLUGINSPATH}"
mkdir -p "${TARGET_MODULEPATH}"
mkdir -p "${TARGET_SCRIPTSPATH}"
mkdir -p "${TARGET_BINARIESPATH}"

markEndOfBlock "Ensure directories exist"

markStartOfBlock "Copy build artifacts to the install path"

cp -r "${BUILT_PLUGINSPATH}" "${TARGET_PLUGINSPATH}"
cp -r "${BUILT_MODULEPATH}" "${TARGET_MODULEPATH}"
cp -r "${BUILT_SCRIPTSPATH}" "${TARGET_SCRIPTSPATH}"
cp -r "${BUILT_BINARIESPATH}" "${TARGET_BINARIESPATH}"

markEndOfBlock "Copy build artifacts to the install path"

markEndOfBlock "$0"
