#!/usr/bin/env bash

pushd "$(dirname "$0")"

    UNREAL_PATH="${1:-"$(pwd)/../../UnrealEngine"}"
    BUILD_PROJECT="${2:-GDKTestGyms}"

    PROJECT_ABSOLUTE_PATH="$(pwd)/../../${BUILD_PROJECT}"
    GDK_IN_TEST_REPO="${PROJECT_ABSOLUTE_PATH}/Game/Plugins/UnrealGDK"

    # Clean up any runtime processes that may not have been shut down
    pkill -9 -f runtime

    rm -rf ${UNREAL_PATH}
    rm -rf ${GDK_IN_TEST_REPO}
    rm -rf ${PROJECT_ABSOLUTE_PATH}
popd
