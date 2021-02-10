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

    # Resolve the TestGym branch to run against. The order of priority is:
    # TEST_REPO_BRANCH envvar > same-name branch as the branch we are currently on > UnrealGDKTestGymsVersion.txt > "master".
    TEST_REPO_BRANCH_LOCAL="${TEST_REPO_BRANCH:-}"
    if [ -z "${TEST_REPO_BRANCH_LOCAL}" ]; then
        TEST_REPO_HEADS=$(git ls-remote --heads "${TEST_REPO_URL}" "${BUILDKITE_BRANCH}")
        GDK_REPO_HEAD="refs/heads/${BUILDKITE_BRANCH}"
        if echo "${TEST_REPO_HEADS}" | grep -qF "${GDK_REPO_HEAD}"; then
            TEST_REPO_BRANCH_LOCAL="${BUILDKITE_BRANCH}"
        else
            # This is a slight hack, we rely on the fact that this version should work even if it's not the TestGym repo
            # (also currently this script only operates on the TestGym repo)
            TESTGYM_VERSION=$(cat "${GDK_HOME}/UnrealGDKTestGymsVersion.txt")
            if [ -z "${TESTGYM_VERSION}" ]; then
                TEST_REPO_BRANCH_LOCAL="master"
            else
                TEST_REPO_BRANCH_LOCAL="${TESTGYM_VERSION}"
            fi
        fi
    fi

    CHOSEN_TEST_REPO_BRANCH="${TEST_REPO_BRANCH_LOCAL}"

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
        "SpatialGDK.+/Game/Intermediate/Maps/CI_Premerge/+/Game/Intermediate/Maps/CI_Premerge_Spatial_Only/" \
        "True"

    if [[ -n "${SLOW_NETWORKING_TESTS}" ]]; then
        echo "--- run-slow-networking-tests"
        "${GDK_HOME}/ci/run-tests.sh" \
            "${UNREAL_PATH}" \
            "${TEST_PROJECT_NAME}" \
            "${UPROJECT_PATH}" \
            "SlowTestResults" \
            "${TEST_REPO_MAP}" \
            "SpatialGDKSlow.+/Game/Intermediate/Maps/CI_Nightly/+/Game/Intermediate/Maps/CI_Nightly_Spatial_Only/" \
            "True"
    fi
popd
