#!/usr/bin/env bash

set -e -u -x -o pipefail

cd "$(dirname "$0")/../"

source ci/pinned-tools.sh
source ci/profiling.sh
source ci/force_spatial_cli_structure.sh

if ! isWindows ; then
  echo "Unreal Sdk is only supported on Windows."
  exit 0
fi

markStartOfBlock "$0"

#####
# Setup variables.
#####
markStartOfBlock "Setup variables"

PINNED_CORE_SDK_VERSION="$(cat core-sdk.version)"

BUILD_DIR="$(pwd)/build"
CORE_SDK_DIR="${BUILD_DIR}/core_sdk"
UNREAL_GDK_DIR="$(pwd)"

markEndOfBlock "Setup variables"

#####
# Clean folders.
#####
markStartOfBlock "Clean folders"

rm -rf "${BUILD_DIR}"
rm -rf "${UNREAL_GDK_DIR}/Source/SpatialGDK/Public/WorkerSdk"

markEndOfBlock "Clean folders"

#####
# Create folders.
#####
markStartOfBlock "Create folders"

mkdir -p "${UNREAL_GDK_DIR}/Source/SpatialGDK/Public/WorkerSdk"

markEndOfBlock "Create folders"

#####
# Retrieve dependencies.
# #####
markStartOfBlock "Retrieve dependencies"

runSpatial worker_package unpack-to worker_sdk core-dynamic-x86-win32 "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Win32"
runSpatial worker_package unpack-to worker_sdk core-dynamic-x86_64-win32 "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Win64"
runSpatial worker_package unpack-to worker_sdk core-dynamic-x86_64-linux "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Linux"

# Download the C++ SDK for its headers, only.
runSpatial worker_package unpack-to worker_sdk cpp-static-x86_64-msvc_mtd-win32 "${CORE_SDK_DIR}/cpp-src"

markEndOfBlock "Retrieve dependencies"

#####
# Unpack dependencies.
#####
markStartOfBlock "Unpack dependencies"

# Include the WorkerSdk header files
cp -r "${CORE_SDK_DIR}/cpp-src/include/"               "${UNREAL_GDK_DIR}/Source/SpatialGDK/Public/WorkerSdk"

markEndOfBlock "Unpack dependencies"

#####
# Build go CLI tools.
#####
markStartOfBlock "Build go CLI tools"

GOOS="windows" GOARCH="amd64" \
  go build -o "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Programs/unreal_packager.exe" improbable.io/unreal_packager/...

GOOS="windows" GOARCH="amd64" \
  go build -o "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Programs/clean.exe" improbable.io/clean/...

markEndOfBlock "Build go CLI tools"

#####
# Process build scripts.
#####

markStartOfBlock "Process build scripts"

cp -a "build_scripts/."                                                     "${UNREAL_GDK_DIR}/Scripts"
sed -i "s/%core_sdk_version%/${PINNED_CORE_SDK_VERSION}/g"                 "${UNREAL_GDK_DIR}/Scripts/"spatialos.*.build.json

markEndOfBlock "Process build scripts"

markEndOfBlock "$0"
