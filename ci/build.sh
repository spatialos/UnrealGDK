#!/usr/bin/env bash

set -e -u -x -o pipefail

cd "$(dirname "$0")/../"

source ci/pinned-tools.sh
source ci/profiling.sh
source ci/force_spatial_cli_structure.sh

if ! isWindows ; then
  echo "Unreal GDK is only supported on Windows."
  exit 0
fi

markStartOfBlock "$0"

#####
# Setup variables.
#####
markStartOfBlock "Setup variables"

PINNED_CORE_SDK_VERSION="$(cat core-sdk.version)"
PINNED_CODE_GENERATOR_VERSION="$(cat code-generator.version)"

UNREAL_GDK_DIR="$(pwd)"
BUILD_DIR="$(pwd)/build"
CORE_SDK_DIR="${BUILD_DIR}/core_sdk"
CODE_GENERATION_DIR="${BUILD_DIR}/code_generation"

PACKAGE_TARGET_DIR="packages"
CACHE_PATH="$HOME/.imp_nuget"
GO_CLI_TOOLS="$(pwd)/build/bin"
UNREAL_CODEGEN_ROOT="Source/Programs/Improbable.Unreal.CodeGeneration/bin/Release"
UNREAL_CODEGEN="${UNREAL_CODEGEN_ROOT}/UnrealCodeGenerator.exe"
markEndOfBlock "Setup variables"

#####
# Clean folders.
#####
markStartOfBlock "Clean folders"

rm -rf "${BUILD_DIR}"
rm -rf "${UNREAL_GDK_DIR}/packages"
rm -rf "${UNREAL_GDK_DIR}/Source/SpatialGDK/Public/WorkerSdk"

markEndOfBlock "Clean folders"

#####
# Create folders.
#####
markStartOfBlock "Create folders"

mkdir -p "${UNREAL_GDK_DIR}/packages"
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

runSpatial worker_package unpack-to code_generation Improbable.CodeGeneration "${PACKAGE_TARGET_DIR}/Improbable.CodeGeneration" --version="${PINNED_CODE_GENERATOR_VERSION}"

markEndOfBlock "Retrieve dependencies"

#####
# Unpack dependencies.
#####
markStartOfBlock "Unpack dependencies"

# Include the WorkerSdk header files
cp -r "${CORE_SDK_DIR}/cpp-src/include/"               "${UNREAL_GDK_DIR}/Source/SpatialGDK/Public/WorkerSdk"

"${IMP_NUGET}" restore-package --cache-directory="${CACHE_PATH}" --target-directory="${PACKAGE_TARGET_DIR}" --package NUnit --version 3.9.0
"${IMP_NUGET}" restore-package --cache-directory="${CACHE_PATH}" --target-directory="${PACKAGE_TARGET_DIR}" --package NUnit.Console --version 3.8.0
"${IMP_NUGET}" restore-package --cache-directory="${CACHE_PATH}" --target-directory="${PACKAGE_TARGET_DIR}" --package NUnit.ConsoleRunner --version 3.8.0
"${IMP_NUGET}" restore-package --cache-directory="${CACHE_PATH}" --target-directory="${PACKAGE_TARGET_DIR}" --package NUnit.Extension.NUnitProjectLoader --version 3.5.0
"${IMP_NUGET}" restore-package --cache-directory="${CACHE_PATH}" --target-directory="${PACKAGE_TARGET_DIR}" --package NUnit.Extension.NUnitV2Driver --version 3.6.0
"${IMP_NUGET}" restore-package --cache-directory="${CACHE_PATH}" --target-directory="${PACKAGE_TARGET_DIR}" --package NUnit.Extension.NUnitV2ResultWriter --version 3.5.0
"${IMP_NUGET}" restore-package --cache-directory="${CACHE_PATH}" --target-directory="${PACKAGE_TARGET_DIR}" --package NUnit.Extension.TeamCityEventListener --version 1.0.3
"${IMP_NUGET}" restore-package --cache-directory="${CACHE_PATH}" --target-directory="${PACKAGE_TARGET_DIR}" --package NUnit.Extension.VSProjectLoader --version 3.7.0

markEndOfBlock "Unpack dependencies"

#####
# Build go CLI tools.
#####
markStartOfBlock "Build go CLI tools"

GOOS="windows" GOARCH="amd64" \
  go build -ldflags="-s -w" -o "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Programs/unreal_packager.exe" improbable.io/unreal_packager/...

GOOS="windows" GOARCH="amd64" \
  go build -ldflags="-s -w" -o "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Programs/clean.exe" improbable.io/clean/...

markEndOfBlock "Build go CLI tools"

#####
# Build CodeGeneration.
#####
markStartOfBlock "Build CodeGeneration"

"${MSBUILD}" "Source/Programs/Improbable.Unreal.CodeGeneration/UnrealCodeGeneration.sln" \
    /property:Configuration='Release' \
    /property:SolutionDir="../" \
    /verbosity:minimal

cp -a "Source/Programs/Improbable.Unreal.CodeGeneration/bin/Release/"*.dll "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Programs"
cp -a "Source/Programs/Improbable.Unreal.CodeGeneration/bin/Release/"*.exe "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Programs"

markEndOfBlock "Build CodeGeneration"

#####
# Process build scripts.
#####

markStartOfBlock "Process build scripts"

cp -a "build_scripts/."                                                    "${UNREAL_GDK_DIR}/Scripts"
sed -i "s/%core_sdk_version%/${PINNED_CORE_SDK_VERSION}/g"                 "${UNREAL_GDK_DIR}/Scripts/"spatialos.*.build.json

markEndOfBlock "Process build scripts"

markEndOfBlock "$0"
