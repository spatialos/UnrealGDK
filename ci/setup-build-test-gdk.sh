#!/usr/bin/env bash

set -e -u -o pipefail
if [[ -n "${DEBUG-}" ]]; then
    set -x
fi

source /opt/improbable/environment

# Detection code copied from ci folder from UnrealGDKExampleProject 
# Resolve the GDK branch to run against. The order of priority is:
# GDK_BRANCH envvar > same-name branch as the branch we are currently on > UnrealGDKVersion.txt > "master".
GDK_BRANCH_LOCAL="${TEST_REPO_BRANCH:-}"
if [ -z "${GDK_BRANCH_LOCAL}" ]; then
    GDK_REPO_HEADS=$(git ls-remote --heads "git@github.com:spatialos/UnrealGDK.git" "${BUILDKITE_BRANCH}")
    TEST_REPO_HEAD="refs/heads/${BUILDKITE_BRANCH}"
    if echo "${GDK_REPO_HEADS}" | grep -qF "${TEST_REPO_HEAD}"; then
        GDK_BRANCH_LOCAL="${BUILDKITE_BRANCH}"
    else
        GDK_VERSION=$(cat UnrealGDKVersion.txt)
        if [ -z "${GDK_VERSION}" ]; then
            GDK_BRANCH_LOCAL="master"
        else
            GDK_BRANCH_LOCAL="${GDK_VERSION}"
        fi
    fi
fi

pushd "$(dirname "$0")"
    GDK_HOME="${1:-"$(pwd)/.."}"
    GCS_PUBLISH_BUCKET="${2:-io-internal-infra-unreal-artifacts-production/UnrealEngine}"
    BUILD_HOME="${3:-"$(pwd)/../.."}"

    UNREAL_PATH="${BUILD_HOME}/UnrealEngine"
    TEST_UPROJECT_NAME="GDKTestGyms"
    TEST_REPO_URL="git@github.com:spatialos/UnrealGDKTestGyms.git"
    TEST_REPO_MAP="EmptyGym"
    TEST_PROJECT_NAME="GDKTestGyms"
    CHOSEN_TEST_REPO_BRANCH="${GDK_BRANCH_LOCAL}"

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
