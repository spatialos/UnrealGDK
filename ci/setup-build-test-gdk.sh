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
    TEST_UPROJECT_NAME="GDKTestGyms"
    TEST_REPO_URL="git@github.com:spatialos/UnrealGDKTestGyms.git"
    TEST_REPO_MAP="EmptyGym"
    TEST_PROJECT_NAME="GDKTestGyms"
    CHOSEN_TEST_REPO_BRANCH="${TEST_REPO_BRANCH:-master}"

    # Download Unreal Engine
    echo "--- get-unreal-engine"
    "${GDK_HOME}/ci/get-engine.sh" \
        "${UNREAL_PATH}" \
        "${GCS_PUBLISH_BUCKET}"

    # Run the required setup steps
    echo "--- setup-gdk"
    "${GDK_HOME}/Setup.sh" --mobile

    # Build the testing project
    UPROJECT_PATH="${BUILD_HOME}/${TEST_PROJECT_NAME}/Game/${TEST_UPROJECT_NAME}.uproject"

    echo "--- build-project"
    "${GDK_HOME}"/ci/build-project.sh \
        "${UNREAL_PATH}" \
        "${CHOSEN_TEST_REPO_BRANCH}" \
        "${TEST_REPO_URL}" \
        "${UPROJECT_PATH}" \
        "${BUILD_HOME}/${TEST_PROJECT_NAME}" \
        "${GDK_HOME}" \
        "${BUILD_PLATFORM}" \
        "${BUILD_STATE}" \
        "${TEST_UPROJECT_NAME}${BUILD_TARGET}"

    echo "--- run-fast-tests"
    "${GDK_HOME}/ci/run-tests.sh" \
        "${UNREAL_PATH}" \
        "${TEST_PROJECT_NAME}" \
        "${UPROJECT_PATH}" \
        "TestResults" \
        "${TEST_REPO_MAP}" \
        "SpatialGDK." \
        "True"

    if [[ -n "${SLOW_NETWORKING_TESTS}" ]]; then
        echo "--- run-slow-networking-tests"
        "${GDK_HOME}/ci/run-tests.sh" \
            "${UNREAL_PATH}" \
            "${TEST_PROJECT_NAME}" \
            "${UPROJECT_PATH}" \
            "SlowTestResults" \
            "${TEST_REPO_MAP}" \
            "SpatialGDKSlow." \
            "True"
    fi
popd
