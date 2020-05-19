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

if [ REPO_NAME!="unrealengine" &&  TEST_NAME!="premerge"]
then
echo "    async: true"
fi

echo "    build:"
echo "      branch: "${BRANCH_TO_TEST}""
echo "      commit: "HEAD""
echo "      env:"

for element in "${ENVIRONMENT_VARIABLES[@]}"
    do
        echo "        ${element}"
    done
}

### unrealengine-premerge
while IFS= read -r ENGINE_VERSION; do
  triggerTest   "unrealengine" \
                "premerge" \
                "${ENGINE_VERSION}-${GDK_VERSION}-rc"
done <<< "${ENGINE_VERSIONS}"

### unrealengine-nightly
while IFS= read -r ENGINE_VERSION; do
  triggerTest   "unrealengine" \
                "nightly" \
                "${ENGINE_VERSION}-${GDK_VERSION}-rc" \
                "GDK_BRANCH: "${GDK_VERSION}-rc"" \
                "EXAMPLE_PROJECT_BRANCH: "${GDK_VERSION}-rc""
done <<< "${ENGINE_VERSIONS}"

### unrealgdk-premerge with SLOW_NETWORKING_TESTS=true
while IFS= read -r ENGINE_VERSION; do
    triggerTest "unrealgdk" \
                "premerge" \
                "${GDK_VERSION}-rc" \
                "SLOW_NETWORKING_TESTS: "true"" \
                "TEST_REPO_BRANCH: "${GDK_VERSION}-rc"" \
                "ENGINE_VERSION: "UnrealEngine-${ENGINE_VERSION}-${GDK_VERSION}-rc""
done <<< "${ENGINE_VERSIONS}"

### unrealgdk-premerge with BUILD_ALL_CONFIGURATIONS=true
while IFS= read -r ENGINE_VERSION; do
    triggerTest "unrealgdk" \
                "premerge" \
                "${GDK_VERSION}-rc" \
                "BUILD_ALL_CONFIGURATIONS: "true"" \
                "TEST_REPO_BRANCH: "${GDK_VERSION}-rc"" \
                "ENGINE_VERSION: "UnrealEngine-${ENGINE_VERSION}-${GDK_VERSION}-rc""
done <<< "${ENGINE_VERSIONS}"

### unrealgdkexampleproject-nightly
while IFS= read -r ENGINE_VERSION; do
    triggerTest "unrealgdkexampleproject" \
                "nightly" \
                "${GDK_VERSION}-rc" \
                "GDK_BRANCH: "${GDK_VERSION}-rc"" \
                "ENGINE_VERSION: "UnrealEngine-${ENGINE_VERSION}-${GDK_VERSION}-rc""
done <<< "${ENGINE_VERSIONS}"

### Commented this out as to not run supefluous large builds during testing.
### TODO: uncomment this
### unrealgdk-nfr
###while IFS= read -r ENGINE_VERSION; do
###    triggerTest "unrealgdk" \
###                "nfr" \
###                "${GDK_VERSION}-rc"
###done <<< "${ENGINE_VERSIONS}"
