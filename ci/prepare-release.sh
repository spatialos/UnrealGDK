#!/usr/bin/env bash

### This script should only be run on Improbable's internal build machines.
### If you don't work at Improbable, this may be interesting as a guide to what software versions we use for our
### automation, but not much more than that.

prepareRelease () {
  local REPO_NAME="${1}"
  local SOURCE_BRANCH="${2}"
  local CANDIDATE_BRANCH="${3}"
  local RELEASE_BRANCH="${4}"
  local GITHUB_ORG="${5}"
  
  echo "--- Preparing ${REPO_NAME}: Cutting ${CANDIDATE_BRANCH} from ${SOURCE_BRANCH}, and creating a PR into ${TARGET_BRANCH} :package:"

  docker run \
    -v "${SECRETS_DIR}":/var/ssh \
    -v "${SECRETS_DIR}":/var/github \
    -v "$(pwd)"/logs:/var/logs \
    local:gdk-release-tool \
        prep "${GDK_VERSION}" \
        --source-branch="${SOURCE_BRANCH}" \
        --candidate-branch="${CANDIDATE_BRANCH}" \
        --release-branch="${RELEASE_BRANCH}" \
        --git-repository-name="${REPO_NAME}" \
        --github-key-file="/var/github/github_token" \
        --github-organization="${GITHUB_ORG}"
}

set -e -u -o pipefail

if [[ -n "${DEBUG-}" ]]; then
  set -x
fi

if [[ -z "$BUILDKITE" ]]; then
  echo "This script is only intended to be run on Improbable CI."
  exit 1
fi

cd "$(dirname "$0")/../"

source ci/common-release.sh

setupReleaseTool

mkdir -p ./logs

# This assigns the gdk-version key that was set in .buildkite\release.steps.yaml to the variable GDK-VERSION
GDK_VERSION="$(buildkite-agent meta-data get gdk-version)"

# This assigns the engine-version key that was set in .buildkite\release.steps.yaml to the variable ENGINE-VERSION
ENGINE_VERSIONS="$(buildkite-agent meta-data get engine-version)"

# Run the C Sharp Release Tool for each candidate we want to cut.
prepareRelease "UnrealGDK" "master" "${GDK_VERSION}-rc" "release" "spatialos"
prepareRelease "UnrealGDKExampleProject" "master" "${GDK_VERSION}-rc" "release" "spatialos"
prepareRelease "UnrealGDKTestGyms" "master" "${GDK_VERSION}-rc" "release" "spatialos"
prepareRelease "UnrealGDKEngineNetTest" "master" "${GDK_VERSION}-rc" "release" "improbable"

while IFS= read -r ENGINE_VERSION; do
  prepareRelease "UnrealEngine" \
    "${ENGINE_VERSION}" \
    "${ENGINE_VERSION}-${GDK_VERSION}-rc" \
    "${ENGINE_VERSION}-release" \
    "improbableio"
done <<< "${ENGINE_VERSIONS}"
