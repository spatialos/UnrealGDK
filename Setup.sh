#!/bin/bash

# EXPERIMENTAL!
# This file is experimental and is not maintained directly by Improbable. Please use at your own risk.

set -e -u -o pipefail

if [ "$(uname -s)" != "Darwin" ]; then
    echo "This script should only be used on OS X. If you are using Windows, please run Setup.bat."
    exit 1
fi

function markStartOfBlock {
    echo "Starting: $1"
}

function markEndOfBlock {
    echo "Finished: $1"
}

pushd "$(dirname "$0")"

markStartOfBlock "$0"

markStartOfBlock "Setup the git hooks"
    if [ -e .git/hooks ]; then
        # Remove the old post-checkout hook.
        if [ -e .git/hooks/post-checkout ]; then rm -f .git/hooks/post-checkout; fi

        # Remove the old post-merge hook.
        if [ -e .git/hooks/post-merge ]; then rm -f .git/hooks/post-merge; fi

        # Add git hook to run Setup.bat when RequireSetup file has been updated.
        echo '#!/usr/bin/env bash' > .git/hooks/post-merge
        echo 'changed_files="$(git diff-tree -r --name-only --no-commit-id ORIG_HEAD HEAD)"' >> .git/hooks/post-merge
        echo 'check_run() {' >> .git/hooks/post-merge
        echo 'echo "$changed_files" | grep --quiet "$1" && exec $2' >> .git/hooks/post-merge
        echo '}' >> .git/hooks/post-merge
        echo 'check_run RequireSetup "sh Setup.sh"' >> .git/hooks/post-merge
    fi
markEndOfBlock "Setup the git hooks"

markStartOfBlock "Check dependencies"
    if [ -z "${UNREAL_HOME:-}" ]; then
        echo "Error: Please set UNREAL_HOME environment variable in ~/.bashrc or ~/.zshrc to point to the Unreal Engine folder."
        exit 1
    fi

    which msbuild > /dev/null
    if [ $? -eq 1 ]; then
        echo "Error: Could not find the MSBuild executable. Please make sure you have Microsoft Visual Studio or Microsoft Build Tools installed."
        exit 1
    fi

    which spatial > /dev/null
    if [ $? -eq 1 ]; then
        echo "Error: Could not find spatial. Please make sure you have it installed and the containing directory added to PATH environment variable."
        exit 1
    fi
markEndOfBlock "Check dependencies"

markStartOfBlock "Setup variables"
    PINNED_CORE_SDK_VERSION=$(cat ./SpatialGDK/Extras/core-sdk.version)
    BUILD_DIR="$(dirname "$0")/SpatialGDK/Build"
    CORE_SDK_DIR="$BUILD_DIR/core_sdk"
    WORKER_SDK_DIR="$(dirname "$0")/SpatialGDK/Source/SpatialGDK/Public/WorkerSDK"
    BINARIES_DIR="$(dirname "$0")/SpatialGDK/Binaries/ThirdParty/Improbable"
    SCHEMA_COPY_DIR="$(dirname "$0")/../../../spatial/schema/unreal/gdk"
    SCHEMA_STD_COPY_DIR="$(dirname "$0")/../../../spatial/build/dependencies/schema/standard_library"
markEndOfBlock "Setup variables"

markStartOfBlock "Clean folders"
    rm -rf $CORE_SDK_DIR           2>/dev/null
    rm -rf $WORKER_SDK_DIR         2>/dev/null
    rm -rf $BINARIES_DIR           2>/dev/null
    rm -rf $SCHEMA_STD_COPY_DIR    2>/dev/null
markEndOfBlock "Clean folders"

markStartOfBlock "Create folders"
    mkdir -p $WORKER_SDK_DIR          >/dev/null 2>/dev/null
    mkdir -p $CORE_SDK_DIR/schema     >/dev/null 2>/dev/null
    mkdir -p $CORE_SDK_DIR/tools      >/dev/null 2>/dev/null
    mkdir -p $CORE_SDK_DIR/worker_sdk >/dev/null 2>/dev/null
    mkdir -p $BINARIES_DIR            >/dev/null 2>/dev/null
    mkdir -p $SCHEMA_STD_COPY_DIR     >/dev/null 2>/dev/null
markEndOfBlock "Create folders"

markStartOfBlock "Retrieve dependencies"
    spatial package retrieve tools           schema_compiler-x86_64-win32               $PINNED_CORE_SDK_VERSION       $CORE_SDK_DIR/tools/schema_compiler-x86_64-win32.zip
    spatial package retrieve schema          standard_library                           $PINNED_CORE_SDK_VERSION       $CORE_SDK_DIR/schema/standard_library.zip
    spatial package retrieve worker_sdk      c-dynamic-x86-msvc_md-win32                $PINNED_CORE_SDK_VERSION       $CORE_SDK_DIR/worker_sdk/c-dynamic-x86-msvc_md-win32.zip
    spatial package retrieve worker_sdk      c-dynamic-x86_64-msvc_md-win32             $PINNED_CORE_SDK_VERSION       $CORE_SDK_DIR/worker_sdk/c-dynamic-x86_64-msvc_md-win32.zip
    spatial package retrieve worker_sdk      c-dynamic-x86_64-gcc_libstdcpp-linux       $PINNED_CORE_SDK_VERSION       $CORE_SDK_DIR/worker_sdk/c-dynamic-x86_64-gcc_libstdcpp-linux.zip
    spatial package retrieve worker_sdk      c-dynamic-x86_64-clang_libcpp-macos        $PINNED_CORE_SDK_VERSION       $CORE_SDK_DIR/worker_sdk/c-dynamic-x86_64-clang_libcpp-macos.zip
    spatial package retrieve worker_sdk      c-static-fullylinked-arm-clang_libcpp-ios  $PINNED_CORE_SDK_VERSION       $CORE_SDK_DIR/worker_sdk/c-static-fullylinked-arm-clang_libcpp-ios.zip
markEndOfBlock "Retrieve dependencies"

markStartOfBlock "Unpack dependencies"
    unzip -oq $CORE_SDK_DIR/worker_sdk/c-dynamic-x86-msvc_md-win32.zip                 -d $BINARIES_DIR/Win32/
    unzip -oq $CORE_SDK_DIR/worker_sdk/c-dynamic-x86_64-msvc_md-win32.zip              -d $BINARIES_DIR/Win64/
    unzip -oq $CORE_SDK_DIR/worker_sdk/c-dynamic-x86_64-gcc_libstdcpp-linux.zip        -d $BINARIES_DIR/Linux/
    unzip -oq $CORE_SDK_DIR/worker_sdk/c-dynamic-x86_64-clang_libcpp-macos.zip         -d $BINARIES_DIR/Mac/
    unzip -oq $CORE_SDK_DIR/worker_sdk/c-static-fullylinked-arm-clang_libcpp-ios.zip   -d $BINARIES_DIR/IOS/
    unzip -oq $CORE_SDK_DIR/tools/schema_compiler-x86_64-win32.zip                     -d $BINARIES_DIR/Programs/
    unzip -oq $CORE_SDK_DIR/schema/standard_library.zip                                -d $BINARIES_DIR/Programs/schema/

    cp -R $BINARIES_DIR/Mac/include/ $WORKER_SDK_DIR
markEndOfBlock "Unpack dependencies"

markStartOfBlock "Copy standard library schema"
    echo "Copying standard library schemas to $SCHEMA_STD_COPY_DIR"
    cp -R $BINARIES_DIR/Programs/schema/* $SCHEMA_STD_COPY_DIR
markEndOfBlock "Copy standard library schema"

markStartOfBlock "Copy GDK schema"
    rm -rf $SCHEMA_COPY_DIR   2>/dev/null
    mkdir -p $SCHEMA_COPY_DIR >/dev/null 2>/dev/null

    echo "Copying schemas to $SCHEMA_COPY_DIR."
    cp -R $(dirname %0)/SpatialGDK/Extras/schema/* $SCHEMA_COPY_DIR
markEndOfBlock "Copy GDK schema"

markStartOfBlock "Build C# utilities"
    msbuild /nologo /verbosity:minimal ./SpatialGDK/Build/Programs/Improbable.Unreal.Scripts/Mac/Improbable.Unreal.Scripts.sln /property:Configuration=Release
markEndOfBlock "Build C# utilities"

markEndOfBlock "$0"

popd

echo "UnrealGDK build completed successfully!"
exit 0
