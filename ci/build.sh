#!/usr/bin/env bash

set -e -u -x -o pipefail

cd "$(dirname "$0")/../"

source ci/pinned-tools.sh
source ci/profiling.sh

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

PACKAGE_TARGET_DIR="packages"
CACHE_PATH="$HOME/.imp_nuget"
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
mkdir -p "${CORE_SDK_DIR}/schema"
mkdir -p "${CORE_SDK_DIR}/tools"
mkdir -p "${CORE_SDK_DIR}/worker_sdk"
mkdir -p "${BUILD_DIR}/code_generation"

markEndOfBlock "Create folders"

#####
# Retrieve dependencies.
# #####
markStartOfBlock "Retrieve dependencies"

spatial package retrieve "tools"            "schema_compiler-x86_64-win32"        "${PINNED_CORE_SDK_VERSION}"       "${CORE_SDK_DIR}/tools/schema_compiler-x86_64-win32"
spatial package retrieve "schema"           "standard_library"                    "${PINNED_CORE_SDK_VERSION}"       "${CORE_SDK_DIR}/schema/standard_library"
spatial package retrieve "worker_sdk"       "core-dynamic-x86-win32"              "${PINNED_CORE_SDK_VERSION}"       "${CORE_SDK_DIR}/worker_sdk/core-dynamic-x86-win32"
spatial package retrieve "worker_sdk"       "core-dynamic-x86_64-win32"           "${PINNED_CORE_SDK_VERSION}"       "${CORE_SDK_DIR}/worker_sdk/core-dynamic-x86_64-win32"
spatial package retrieve "worker_sdk"       "core-dynamic-x86_64-linux"           "${PINNED_CORE_SDK_VERSION}"       "${CORE_SDK_DIR}/worker_sdk/core-dynamic-x86_64-linux"
spatial package retrieve "code_generation"  "Improbable.CodeGeneration"           "${PINNED_CODE_GENERATOR_VERSION}" "${BUILD_DIR}/code_generation/Improbable.CodeGeneration"
# Download the C++ SDK for its headers, only.
spatial package retrieve "worker_sdk"      "cpp-static-x86_64-msvc_mtd-win32"     "${PINNED_CORE_SDK_VERSION}"       "${CORE_SDK_DIR}/cpp-static-x86_64-msvc_mtd-win32"

markEndOfBlock "Retrieve dependencies"

#####
# Unpack dependencies.
#####
markStartOfBlock "Unpack dependencies"

# Include the WorkerSdk header files
unpackTo "${CORE_SDK_DIR}/cpp-static-x86_64-msvc_mtd-win32" "${CORE_SDK_DIR}/cpp-src"
rm -rf "${UNREAL_GDK_DIR}/Source/SpatialGDK/Public/WorkerSdk"
cp -r "${CORE_SDK_DIR}/cpp-src/include" "${UNREAL_GDK_DIR}/Source/SpatialGDK/Public/WorkerSdk"

unpackToWithClean "${CORE_SDK_DIR}/worker_sdk/core-dynamic-x86-win32"       "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Win32"
unpackToWithClean "${CORE_SDK_DIR}/worker_sdk/core-dynamic-x86_64-win32"    "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Win64"
unpackToWithClean "${CORE_SDK_DIR}/worker_sdk/core-dynamic-x86_64-linux"    "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Linux"
unpackTo          "${CORE_SDK_DIR}/tools/schema_compiler-x86_64-win32"      "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Programs"
unpackTo          "${CORE_SDK_DIR}/schema/standard_library"                 "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Programs/schema"
unpackTo          "${BUILD_DIR}/code_generation//Improbable.CodeGeneration" "${PACKAGE_TARGET_DIR}/Improbable.CodeGeneration"

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
# Build CodeGeneration.
#####
markStartOfBlock "Build CodeGeneration"

"${MSBUILD}" "Source/Programs/Improbable.Unreal.CodeGeneration/UnrealCodeGeneration.sln" \
    /property:Configuration='Release' \
    /property:SolutionDir="../" \
    /verbosity:minimal

cp -a "Source/Programs/Improbable.Unreal.CodeGeneration/bin/Release/"*.dll "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Programs"
cp -a "Source/Programs/Improbable.Unreal.CodeGeneration/bin/Release/"*.exe "${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Programs"

csc "Scripts/Build.cs" "Scripts/Codegen.cs" "Scripts/Common.cs" -main:"Improbable.Build" -nologo -out:"${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Programs/Build.exe"
csc "Scripts/Codegen.cs" "Scripts/Common.cs"                                             -nologo -out:"${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Programs/Codegen.exe"
csc "Scripts/DiffCopy.cs"                                                                -nologo -out:"${UNREAL_GDK_DIR}/Binaries/ThirdParty/Improbable/Programs/DiffCopy.exe"

markEndOfBlock "Build CodeGeneration"

markEndOfBlock "$0"