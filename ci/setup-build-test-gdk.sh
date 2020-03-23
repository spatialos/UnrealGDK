#!/usr/bin/env bash

set -e -u -o pipefail
if [[ -n "${DEBUG-}" ]]; then
    set -x
fi

source /opt/improbable/environment

pushd "$(dirname "$0")"
    GDK_HOME="${1:-"$(pwd)/.."}"
    GCS_PUBLISH_BUCKET="${2:-io-internal-infra-unreal-artifacts-production/UnrealEngine}"
    BUILD_HOME="${3:-"$(pwd)/../.."}"

    UNREAL_PATH="${BUILD_HOME}/UnrealEngine"
    TEST_REPO_URL="git@github.com:improbable/UnrealGDKEngineNetTest.git"
    TEST_REPO_RELATIVE_UPROJECT_PATH="Game/EngineNetTest.uproject"
    TEST_REPO_MAP="NetworkingMap"
    TEST_PROJECT_NAME="NetworkTestProject"
    CHOSEN_TEST_REPO_BRANCH="master"
    SLOW_NETWORKING_TESTS=false

    # Allow overriding testing branch via environment variable
    if [[ -n "${TEST_REPO_BRANCH:-}" ]]; then
        CHOSEN_TEST_REPO_BRANCH="${TEST_REPO_BRANCH}"
    fi

    # Download Unreal Engine
    echo "--- get-unreal-engine"
    "${GDK_HOME}/ci/get-engine.sh" \
        "${UNREAL_PATH}" \
        "${GCS_PUBLISH_BUCKET}"

    # Run the required setup steps
    echo "--- setup-gdk"
    "${GDK_HOME}/Setup.sh" --mobile

    # Build the testing project
    echo "--- build-project"
    "${GDK_HOME}"/ci/build-project.sh \
        "${UNREAL_PATH}" \
        "${CHOSEN_TEST_REPO_BRANCH}" \
        "${TEST_REPO_URL}" \
        "${BUILD_HOME}/${TEST_PROJECT_NAME}/${TEST_REPO_RELATIVE_UPROJECT_PATH}" \
        "${BUILD_HOME}/${TEST_PROJECT_NAME}" \
        "${GDK_HOME}" \
        "${BUILD_PLATFORM}" \
        "${BUILD_STATE}" \
        "${BUILD_TARGET}"

    echo "--- run-fast-tests"
    "${GDK_HOME}/ci/run-tests.sh" \
        "${UNREAL_PATH}" \
        "${BUILD_HOME}" \
        "${TEST_PROJECT_NAME}" \
        "${TEST_REPO_RELATIVE_UPROJECT_PATH}" \
        "FastTestResults" \
        "SpatialGDK+/Game/SpatialNetworkingMap" \
        "True"

    if [[ -n "${SLOW_NETWORKING_TESTS}" ]]; then
        echo "--- run-slow-networking-tests"
        "${GDK_HOME}/ci/run-tests.sh" \
            "${UNREAL_PATH}" \
            "${BUILD_HOME}" \
            "${TEST_PROJECT_NAME}" \
            "${TEST_REPO_RELATIVE_UPROJECT_PATH}" \
            "VanillaTestResults" \
            "ci/${TEST_REPO_MAP}" \
            "/Game/NetworkingMap" \
            ""
    fi
popd
