#!/usr/bin/env bash

### This script should only be run on Improbable's internal build machines.
### If you don't work at Improbable, this may be interesting as a guide to what software versions we use for our
### automation, but not much more than that.

# exit immediately on failure, or if an undefined variable is used
set -eu

# This assigns the gdk-version key that was set in .buildkite\release.steps.yaml to the variable GDK-VERSION
GDK_VERSION="$(buildkite-agent meta-data get gdk-version)"

# This assigns the engine-version key that was set in .buildkite\release.steps.yaml to the variable ENGINE-VERSION
ENGINE_VERSIONS="$(buildkite-agent meta-data get engine-versions)"

echo "steps:"
triggerTest () {
  local REPO_NAME="${1}"
  local TEST_NAME="${2}"
  local BRANCH_TO_TEST="${3}"
  local ENVIRONMENT_VARIABLES=( "${@:4}" )
  
echo "  - trigger: "${REPO_NAME}-${TEST_NAME}""
echo "    label: "Run ${REPO_NAME}-${TEST_NAME} at HEAD OF ${BRANCH_TO_TEST}""
echo "    build:"
echo "      branch: "${BRANCH_TO_TEST}""
echo "      commit: "HEAD""

### unrealengine-premerge
while IFS= read -r ENGINE_VERSION; do
  triggerTest   "unrealengine" \
                "premerge" \
                "${ENGINE_VERSION}-${GDK_VERSION}-rc"
done <<< "${ENGINE_VERSIONS}"
