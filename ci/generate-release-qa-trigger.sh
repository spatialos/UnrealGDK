#!/usr/bin/env bash

### This script should only be run on Improbable's internal build machines.
### If you don't work at Improbable, this may be interesting as a guide to what software versions we use for our
### automation, but not much more than that.

# exit immediately on failure, or if an undefined variable is used
set -eu

if [[ -n "${DEBUG-}" ]]; then
  set -x
fi

if [[ -z "$BUILDKITE" ]]; then
  echo "This script is only intended to be run on Improbable CI."
  exit 1
fi

### TODO: Grab all candidate branches, feed them into test CI

# This assigns the gdk-version key that was set in .buildkite\release.steps.yaml to the variable GDK-VERSION
GDK_VERSION="$(buildkite-agent meta-data get gdk-version)"

# This assigns the engine-version key that was set in .buildkite\release.steps.yaml to the variable ENGINE-VERSION
ENGINE_VERSIONS="$(buildkite-agent meta-data get engine-version)"

# Repurpoused from prepareRelease
triggerTest () {
  local REPO_NAME="${1}"
  local TEST_NAME="${2}"
  local GDK_BRANCH_TO_TEST="${3}"
  local ENGINE_BRANCH_TO_TEST="${4}"
  
echo "--- Triggering ${REPO_NAME}-${TEST_NAME} at HEAD of ${CANDIDATE_BRANCH}"
echo "steps:"
echo "  - trigger: ${REPO_NAME}-${TEST_NAME}"
echo "    label: Run ${REPO_NAME}-${TEST_NAME} at HEAD OF ${CANDIDATE_BRANCH}"
echo "    build:"
echo "      branch: ${GDK_BRANCH_TO_TEST}"
}

### unrealgdk-premerge
triggerTest "UnrealGDK" "premerge" "${GDK_VERSION}-rc"
### unrealgdkexampleproject-nightly
triggerTest "UnrealGDKExampleProject" "nightly" "${GDK_VERSION}-rc"
### unrealgdk-nfr
triggerTest "UnrealGDKExampleProject" "nfr" "${GDK_VERSION}-rc"

### unrealengine-premerge
while IFS= read -r ENGINE_VERSION; do
  triggerTest "UnrealEngine" \
    "premerge" \
    "${GDK_VERSION}-rc" \
    "${ENGINE_VERSION}-${GDK_VERSION}-rc"
done <<< "${ENGINE_VERSIONS}"

### unrealengine-nightly
while IFS= read -r ENGINE_VERSION; do
  triggerTest "UnrealEngine" \
    "nightly" \
    "${GDK_VERSION}-rc" \
    "${ENGINE_VERSION}-${GDK_VERSION}-rc"
done <<< "${ENGINE_VERSIONS}"
