#!/usr/bin/env bash

set -e -u -o pipefail
[[ -n "${DEBUG:-}" ]] && set -x

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "This script should only be used on OS X. If you are using Windows, please run Setup.bat."
    exit 1
fi

pushd "$(dirname "$0")"

PINNED_CORE_SDK_VERSION=$(cat ./SpatialGDK/Extras/core-sdk.version)
PINNED_SPOT_VERSION=$(cat ./SpatialGDK/Extras/spot.version)
BUILD_DIR="$(pwd)/SpatialGDK/Build"
CORE_SDK_DIR="${BUILD_DIR}/core_sdk"
WORKER_SDK_DIR="$(pwd)/SpatialGDK/Source/SpatialGDK/Public/WorkerSDK"
BINARIES_DIR="$(pwd)/SpatialGDK/Binaries/ThirdParty/Improbable"
SCHEMA_COPY_DIR="$(pwd)/../../../spatial/schema/unreal/gdk"
SCHEMA_STD_COPY_DIR="$(pwd)/../../../spatial/build/dependencies/schema/standard_library"
SPATIAL_DIR="$(pwd)/../../../spatial"


echo "Setup the git hooks"
if [[ -e .git/hooks ]]; then
    # Remove the old post-checkout hook.
    if [[ -e .git/hooks/post-checkout ]]; then
        rm -f .git/hooks/post-checkout
    fi

    # Remove the old post-merge hook.
    if [[ -e .git/hooks/post-merge ]]; then
        rm -f .git/hooks/post-merge
    fi

    # Add git hook to run Setup.sh when RequireSetup file has been updated.
    cp "$(pwd)/SpatialGDK/Extras/git/post-merge" "$(pwd)/.git/hooks"
fi

echo "Clean folders"
rm -rf "${CORE_SDK_DIR}"
rm -rf "${WORKER_SDK_DIR}"
rm -rf "${BINARIES_DIR}"

if [[ -d "${SPATIAL_DIR}" ]]; then
    rm -rf "${SCHEMA_STD_COPY_DIR}"
    rm -rf "${SCHEMA_COPY_DIR}"
fi

echo "Create folders"
mkdir -p "${WORKER_SDK_DIR}"
mkdir -p "${CORE_SDK_DIR}"/schema
mkdir -p "${CORE_SDK_DIR}"/tools
mkdir -p "${CORE_SDK_DIR}"/worker_sdk
mkdir -p "${BINARIES_DIR}"/Programs/worker_sdk

if [ -d "${SPATIAL_DIR}" ]; then
    mkdir -p "${SCHEMA_STD_COPY_DIR}"
    mkdir -p "${SCHEMA_COPY_DIR}"
fi

echo "Retrieve dependencies"
spatial package retrieve tools       schema_compiler-x86_64-macos        "${PINNED_CORE_SDK_VERSION}"  "${CORE_SDK_DIR}"/tools/schema_compiler-x86_64-macos.zip
spatial package retrieve schema      standard_library                    "${PINNED_CORE_SDK_VERSION}"  "${CORE_SDK_DIR}"/schema/standard_library.zip
spatial package retrieve worker_sdk  c_headers                           "${PINNED_CORE_SDK_VERSION}"  "${CORE_SDK_DIR}"/worker_sdk/c_headers.zip
spatial package retrieve worker_sdk  c-dynamic-x86_64-clang-macos        "${PINNED_CORE_SDK_VERSION}"  "${CORE_SDK_DIR}"/worker_sdk/c-dynamic-x86_64-clang-macos.zip
spatial package retrieve worker_sdk  c-static-fullylinked-arm-clang-ios  "${PINNED_CORE_SDK_VERSION}"  "${CORE_SDK_DIR}"/worker_sdk/c-static-fullylinked-arm-clang-ios.zip
spatial package retrieve worker_sdk  csharp                              "${PINNED_CORE_SDK_VERSION}"  "${CORE_SDK_DIR}"/worker_sdk/csharp.zip
spatial package retrieve spot        spot-macos                          "${PINNED_SPOT_VERSION}"      "${BINARIES_DIR}"/Programs/spot
chmod +x "${BINARIES_DIR}"/Programs/spot

echo "Unpack dependencies"
unzip -oq "${CORE_SDK_DIR}"/tools/schema_compiler-x86_64-macos.zip            -d "${BINARIES_DIR}"/Programs/
unzip -oq "${CORE_SDK_DIR}"/schema/standard_library.zip                       -d "${BINARIES_DIR}"/Programs/schema/
unzip -oq "${CORE_SDK_DIR}"/worker_sdk/c_headers.zip                          -d "${BINARIES_DIR}"/Headers/
unzip -oq "${CORE_SDK_DIR}"/worker_sdk/c-dynamic-x86_64-clang-macos.zip       -d "${BINARIES_DIR}"/Mac/
unzip -oq "${CORE_SDK_DIR}"/worker_sdk/c-static-fullylinked-arm-clang-ios.zip -d "${BINARIES_DIR}"/IOS/
unzip -oq "${CORE_SDK_DIR}"/worker_sdk/csharp.zip                             -d "${BINARIES_DIR}"/Programs/worker_sdk/csharp/
cp -R "${BINARIES_DIR}"/Headers/include/ "${WORKER_SDK_DIR}"

if [[ -d "${SPATIAL_DIR}" ]]; then
    echo "Copying standard library schemas to ${SCHEMA_STD_COPY_DIR}"
    cp -R "${BINARIES_DIR}/Programs/schema" "${SCHEMA_STD_COPY_DIR}"

    echo "Copying schemas to ${SCHEMA_COPY_DIR}"
    cp -R SpatialGDK/Extras/schema "${SCHEMA_COPY_DIR}"
fi

popd

echo "UnrealGDK build completed successfully!"
