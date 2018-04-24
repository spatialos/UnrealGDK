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

# ci/linting/test.sh # because fail-fast on obvious syntax errors

#####
# Setup variables.
#####
markStartOfBlock "Setup variables"

#BUILD_NUMBER=$(go run "${GOPATH}/src/improbable.io/get-version-string/main.go")
PINNED_CORE_SDK_VERSION="$(cat core-sdk.version)"
PINNED_CODE_GENERATOR_VERSION="$(cat code-generator.version)"

BUILD_DIR="$(pwd)/build"
CORE_SDK_DIR="${BUILD_DIR}/core_sdk"
# CODE_GENERATION_DIR="${BUILD_DIR}/code_generation"
# WORKER_PACKAGE_DIR="${BUILD_DIR}/worker_packages"

UNREAL_GDK_DIR="$(pwd)"

# SCHEMA_DIR="$(pwd)/schema"
# SCHEMA_STD_DIR="$(pwd)/build/standard_library"
# SCHEMA_COMPILER_DIR="$(pwd)/build/schema_compiler"
# SCHEMA_JSON_DIR="$(pwd)/build/json"

# PACKAGE_TARGET_DIR="packages"
# CACHE_PATH="$HOME/.imp_nuget"
GO_CLI_TOOLS="$(pwd)/build/bin"
# UNREAL_CODEGEN_ROOT="Source/Programs/Improbable.Unreal.CodeGeneration/bin/Release"
# UNREAL_CODEGEN="${UNREAL_CODEGEN_ROOT}/UnrealCodeGenerator.exe"
# UNIFIED_UNREAL_SDK_TEMP_DIR="build/unified_unreal_sdk"

# UNREAL_GENERATED_CODE_DIR="${UNREAL_GDK_DIR}/Source/SpatialOS/Generated"

markEndOfBlock "Setup variables"

#####
# Clean folders.
#####
markStartOfBlock "Clean folders"

rm -rf "${CORE_SDK_DIR}/tools"
rm -rf "${GO_CLI_TOOLS}"
rm -rf "${BUILD_DIR}"
# rm -rf "${UNIFIED_UNREAL_SDK_TEMP_DIR}"
rm -rf "${UNREAL_GDK_DIR}/packages"
# rm -rf "${UNREAL_GDK_DIR}/Source/SpatialOS/Private/WorkerSdk"
rm -rf "${UNREAL_GDK_DIR}/Source/SpatialOS/Public/WorkerSdk"

markEndOfBlock "Clean folders"

#####
# Create folders.
#####
markStartOfBlock "Create folders"

# mkdir -p "${BUILD_DIR}/json"
# mkdir -p "${WORKER_PACKAGE_DIR}"
# mkdir -p "${CODE_GENERATION_DIR}"
# mkdir -p "${CORE_SDK_DIR}/tools"
# mkdir -p "${CORE_SDK_DIR}/schema"
mkdir -p "${CORE_SDK_DIR}/worker_sdk"
# mkdir -p "${UNREAL_GDK_DIR}/packages"
# mkdir -p "${UNREAL_GDK_DIR}/Source/SpatialOS/Private/WorkerSdk"
mkdir -p "${UNREAL_GDK_DIR}/Source/SpatialOS/Public/WorkerSdk"
# mkdir -p "${UNIFIED_UNREAL_SDK_TEMP_DIR}/Game/Source/SpatialOS"
# mkdir -p "${UNIFIED_UNREAL_SDK_TEMP_DIR}/Game/Plugins/SpatialOS"
# mkdir -p "${UNIFIED_UNREAL_SDK_TEMP_DIR}/Game/Binaries/ThirdParty/Improbable/Programs"
# mkdir -p "${UNREAL_GENERATED_CODE_DIR}/Usr"

markEndOfBlock "Create folders"

#####
# Retrieve dependencies.
#####
markStartOfBlock "Retrieve dependencies"

# CoreSDK dependencies.
# "${PACKAGE_CLIENT}" retrieve "tools"      "${SCHEMA_COMPILER_PACKAGE}" "${PINNED_CORE_SDK_VERSION}" "${CORE_SDK_DIR}/tools/${SCHEMA_COMPILER_PACKAGE}"
# "${PACKAGE_CLIENT}" retrieve "schema"     "standard_library"           "${PINNED_CORE_SDK_VERSION}" "${CORE_SDK_DIR}/schema/standard_library"
runSpatial worker_package unpack-to worker_sdk core-dynamic-x86-win32 "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Win32"
runSpatial worker_package unpack-to worker_sdk core-dynamic-x86_64-win32 "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Win64"
runSpatial worker_package unpack-to worker_sdk core-dynamic-x86_64-linux "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Linux"

# Download the C++ SDK for its headers, only.
runSpatial worker_package unpack-to worker_sdk cpp-static-x86_64-msvc_mtd-win32 "${CORE_SDK_DIR}/cpp-src"

# # Engines dependencies.
# if [ "${BUILD_NUMBER}" != "LOCAL" ] ; then
#   "${PACKAGE_CLIENT}" retrieve "code_generation" "Improbable.CodeGeneration" "${PINNED_CODE_GENERATOR_VERSION}" "${CODE_GENERATION_DIR}/Improbable.CodeGeneration"
# else
#   cp "../develop-code-generator/build/worker_packages/code_generation/Improbable.CodeGeneration.zip" "${CODE_GENERATION_DIR}"
# fi

markEndOfBlock "Retrieve dependencies"

#####
# Unpack dependencies.
#####
markStartOfBlock "Unpack dependencies"

# unpackToWithClean "${CORE_SDK_DIR}/schema/standard_library"              "${SCHEMA_STD_DIR}"
# unpackToWithClean "${CORE_SDK_DIR}/tools/${SCHEMA_COMPILER_PACKAGE}"     "${SCHEMA_COMPILER_DIR}"

# Include the WorkerSdk header files
cp -r "${CORE_SDK_DIR}/cpp-src/include/"               "${UNREAL_GDK_DIR}/Source/SpatialGDK/Public/WorkerSdk"

# unpackToWithClean "${CODE_GENERATION_DIR}/Improbable.CodeGeneration" "packages/Improbable.CodeGeneration"

#"${IMP_NUGET}" restore-package --cache-directory="${CACHE_PATH}" --target-directory="${PACKAGE_TARGET_DIR}" --package NUnit --version 3.9.0
# "${IMP_NUGET}" restore-package --cache-directory="${CACHE_PATH}" --target-directory="${PACKAGE_TARGET_DIR}" --package NUnit.Console --version 3.8.0
# "${IMP_NUGET}" restore-package --cache-directory="${CACHE_PATH}" --target-directory="${PACKAGE_TARGET_DIR}" --package NUnit.ConsoleRunner --version 3.8.0
# "${IMP_NUGET}" restore-package --cache-directory="${CACHE_PATH}" --target-directory="${PACKAGE_TARGET_DIR}" --package NUnit.Extension.NUnitProjectLoader --version 3.5.0
# "${IMP_NUGET}" restore-package --cache-directory="${CACHE_PATH}" --target-directory="${PACKAGE_TARGET_DIR}" --package NUnit.Extension.NUnitV2Driver --version 3.6.0
# "${IMP_NUGET}" restore-package --cache-directory="${CACHE_PATH}" --target-directory="${PACKAGE_TARGET_DIR}" --package NUnit.Extension.NUnitV2ResultWriter --version 3.5.0
# "${IMP_NUGET}" restore-package --cache-directory="${CACHE_PATH}" --target-directory="${PACKAGE_TARGET_DIR}" --package NUnit.Extension.TeamCityEventListener --version 1.0.3
# "${IMP_NUGET}" restore-package --cache-directory="${CACHE_PATH}" --target-directory="${PACKAGE_TARGET_DIR}" --package NUnit.Extension.VSProjectLoader --version 3.7.0

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

# #####
# # Build CodeGeneration.
# #####
# markStartOfBlock "Build CodeGeneration"

# "${XBUILD_OR_MSBUILD}" "UnrealCodeGeneration.sln" \
#     /property:Configuration='Release' \
#     /property:SolutionDir="../" \
#     /verbosity:minimal

# markEndOfBlock "Build CodeGeneration"

# # markStartOfBlock "Generate code"

# # "${SCHEMA_COMPILER_DIR}/schema_compiler" \
# #   --schema_path="${SCHEMA_STD_DIR}" \
# #   --schema_path="${SCHEMA_DIR}" \
# #   --ast_json_out="${SCHEMA_JSON_DIR}" \
# #   --cpp_out="${UNREAL_GENERATED_CODE_DIR}/Usr" \
# #   "${SCHEMA_DIR}"/*.schema \
# #   "${SCHEMA_STD_DIR}"/*/*.schema

# # "${UNREAL_CODEGEN}" --json-dir="${SCHEMA_JSON_DIR}" \
# #   --output-dir="${UNREAL_GENERATED_CODE_DIR}/UClasses"

# #####
# # Merge into Unified Unreal SDK.
# #####
# markStartOfBlock "Merge into Unified Unreal SDK"

# # CoreSdk dynamic libraries.
# unpackTo "${CORE_SDK_DIR}/worker_sdk/core-dynamic-x86_64-linux"            "${UNIFIED_UNREAL_SDK_TEMP_DIR}/Game/Binaries/ThirdParty/Improbable/Linux"
# unpackTo "${CORE_SDK_DIR}/worker_sdk/core-dynamic-x86-win32"               "${UNIFIED_UNREAL_SDK_TEMP_DIR}/Game/Binaries/ThirdParty/Improbable/Win32"
# unpackTo "${CORE_SDK_DIR}/worker_sdk/core-dynamic-x86_64-win32"            "${UNIFIED_UNREAL_SDK_TEMP_DIR}/Game/Binaries/ThirdParty/Improbable/Win64"

# # Build scripts.
# cp -a "build_script/."                                                     "${UNIFIED_UNREAL_SDK_TEMP_DIR}/Game/Source/SpatialOS"
# sed -i "s/%core_sdk_version%/${PINNED_CORE_SDK_VERSION}/g"                 "${UNIFIED_UNREAL_SDK_TEMP_DIR}/Game/Source/SpatialOS/"*.json

# # CLI helpers.
# cp -a "${GO_CLI_TOOLS}/windows/."                                          "${UNIFIED_UNREAL_SDK_TEMP_DIR}/Game/Binaries/ThirdParty/Improbable/Programs"
# cp -a "Source/Programs/Improbable.Unreal.CodeGeneration/bin/Release/"*.dll "${UNIFIED_UNREAL_SDK_TEMP_DIR}/Game/Binaries/ThirdParty/Improbable/Programs"
# cp -a "Source/Programs/Improbable.Unreal.CodeGeneration/bin/Release/"*.exe "${UNIFIED_UNREAL_SDK_TEMP_DIR}/Game/Binaries/ThirdParty/Improbable/Programs"

# # Source code.
# # cp -a "Source/SpatialOS/Public/."                                          "${UNIFIED_UNREAL_SDK_TEMP_DIR}/Game/Source/SpatialOS/Public"
# # cp -a "Source/SpatialOS/Private/."                                         "${UNIFIED_UNREAL_SDK_TEMP_DIR}/Game/Source/SpatialOS/Private"
# # cp -a "Source/SpatialOS/SpatialOS.Build.cs"                                "${UNIFIED_UNREAL_SDK_TEMP_DIR}/Game/Source/SpatialOS"
# # cp -a "Plugins/SpatialOS/."                                                "${UNIFIED_UNREAL_SDK_TEMP_DIR}/Game/Plugins/SpatialOS"

# # Docs
# # cp -a "github_docs/."                                                      "${UNIFIED_UNREAL_SDK_TEMP_DIR}/"

# # # .gitattributes.
# # cp "github_gitattributes"                                                  "${UNIFIED_UNREAL_SDK_TEMP_DIR}/.gitattributes"

# # # Package.
# #"${IMP_WORKER_PACKAGE}" pack \
# #   --output="${WORKER_PACKAGE_DIR}/unified_unreal_sdk/unified_unreal_sdk" \
# #   --basePath="${UNIFIED_UNREAL_SDK_TEMP_DIR}" \
# #   "**"
# # "${IMP_WORKER_PACKAGE}" upload "${WORKER_PACKAGE_DIR}" \
# #   --type="unified_unreal_sdk" \
# #   --local \
# #   --version-string="${BUILD_NUMBER}"

# markEndOfBlock "Merge into Unified Unreal SDK"

markEndOfBlock "$0"
