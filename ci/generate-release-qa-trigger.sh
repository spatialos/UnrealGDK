#!/usr/bin/env bash

### This script should only be run on Improbable's internal build machines.
### If you don't work at Improbable, this may be interesting as a guide to what software versions we use for our
### automation, but not much more than that.

# exit immediately on failure, or if an undefined variable is used
set -eu

# This assigns the gdk-version key that was set in .buildkite\release.steps.yaml to the variable GDK-VERSION
GDK_VERSION="$(buildkite-agent meta-data get gdk-version)"

# This assigns the engine-version key that was set in .buildkite\release.steps.yaml to the variable ENGINE-VERSION
ENGINE_VERSIONS="$(buildkite-agent meta-data get engine-version)"

# Repurpoused from prepareRelease
triggerTest () {
  local REPO_NAME="${1}"
  local TEST_NAME="${2}"
  local BRANCH_TO_TEST="${3}"
  local ENVIRONMENT_VARIABLES="${4}"
  
steps:
  - trigger: "${REPO_NAME}-${TEST_NAME}"
    label: "Run ${REPO_NAME}-${TEST_NAME} at HEAD OF ${BRANCH_TO_TEST}"
    async: true
    build:
        branch: "${GDK_BRANCH_TO_TEST}"
        commit: "HEAD"
    env: "${ENVIRONMENT_VARIABLE}"

}

### unrealgdk-premerge with SLOW_NETWORKING_TESTS=true
while IFS= read -r ENGINE_VERSION; do
    triggerTest "UnrealGDK" \
                "premerge" \
                "${GDK_VERSION}-rc" \
                "SLOW_NETWORKING_TESTS: "true" \n
                TEST_REPO_BRANCH: "${GDK_VERSION}-rc" \n
                ENGINE_VERSION: "UnrealEngine-${ENGINE_VERSION}-${GDK_VERSION}-rc""
done <<< "${ENGINE_VERSIONS}"

### unrealgdk-premerge with BUILD_ALL_CONFIGURATIONS=true
while IFS= read -r ENGINE_VERSION; do
    triggerTest "UnrealGDK" \
                "premerge" \
                "${GDK_VERSION}-rc" \
                "BUILD_ALL_CONFIGURATIONS: "true" \n
                TEST_REPO_BRANCH: "${GDK_VERSION}-rc" \n
                ENGINE_VERSION: "UnrealEngine-${ENGINE_VERSION}-${GDK_VERSION}-rc""
done <<< "${ENGINE_VERSIONS}"

### unrealgdkexampleproject-nightly
while IFS= read -r ENGINE_VERSION; do
    triggerTest "UnrealGDKExampleProject" \
                "nightly" \
                "${GDK_VERSION}-rc" \
                "GDK_BRANCH: "${GDK_VERSION}-rc" \n
                ENGINE_VERSION: "UnrealEngine-${ENGINE_VERSION}-${GDK_VERSION}-rc""
done <<< "${ENGINE_VERSIONS}"

### unrealgdk-nfr
while IFS= read -r ENGINE_VERSION; do
    triggerTest "UnrealGDK" \
                "nfr" \
                "${GDK_VERSION}-rc"
done <<< "${ENGINE_VERSIONS}"

### unrealengine-premerge
while IFS= read -r ENGINE_VERSION; do
  triggerTest   "UnrealEngine" \
                "premerge" \
                "${ENGINE_VERSION}-${GDK_VERSION}-rc"
done <<< "${ENGINE_VERSIONS}"

### unrealengine-nightly
while IFS= read -r ENGINE_VERSION; do
  triggerTest   "UnrealEngine" \
                "nightly" \
                "${ENGINE_VERSION}-${GDK_VERSION}-rc"
                "GDK_BRANCH: "${GDK_VERSION}-rc" \n
                EXAMPLE_PROJECT_BRANCH: "${GDK_VERSION}-rc""
done <<< "${ENGINE_VERSIONS}"
