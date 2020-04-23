#!/usr/bin/env bash

### This script should only be run on Improbable's internal build machines.
### If you don't work at Improbable, this may be interesting as a guide to what software versions we use for our
### automation, but not much more than that.

release () {
  local REPO_NAME="${1}"
  local SOURCE_BRANCH="${2}"
  local CANDIDATE_BRANCH="${3}"
  local RELEASE_BRANCH="${4}"
  local PR_URL="${5}"
  
  # TODO: update this logging.
  echo "--- Preparing ${REPO}: Cutting ${CANDIDATE_BRANCH} from ${SOURCE_BRANCH}, and creating a PR into ${TARGET_BRANCH} :package:"

  docker run \
    -v "${SECRETS_DIR}":/var/ssh \
    -v "${SECRETS_DIR}":/var/github \
    -v "$(pwd)"/logs:/var/logs \
    local:gdk-release-tool \
        release "${GDK_VERSION}" \
        --source-branch="${SOURCE_BRANCH}" \
        --candidate-branch="${CANDIDATE_BRANCH}" \
        --release-branch="${RELEASE_BRANCH}" \
        --git-repository-name="${REPO_NAME}" \
        --github-key-file="/var/github/github_token" \
        --buildkite-metadata-path="/var/logs/bk-metadata" \
        --pull-request-url="${PR_URL}"
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

RELEASE_VERSION="$(buildkite-agent meta-data get release-version)"

setupReleaseTool

mkdir -p ./logs

# Run the C Sharp Release Tool for each candidate we want to release.
prepareRelease "UnrealGDK" "master" "${GDK_VERSION}-rc" \
  "release" "$(buildkite-agent meta-data get UnrealGDK-pr-url)"
prepareRelease "UnrealGDKExampleProject" "master" "${GDK_VERSION}-rc" \
  "release" "$(buildkite-agent meta-data get UnrealGDKExampleProject-pr-url)"
prepareRelease "UnrealGDKTestGyms" "master" "${GDK_VERSION}-rc" \
  "release" "$(buildkite-agent meta-data get UnrealGDKTestGyms-pr-url)"
prepareRelease "UnrealGDKEngineNetTest" "master" "${GDK_VERSION}-rc" \
  "release" "$(buildkite-agent meta-data get UnrealGDKEngineNetTest-pr-url)"

while IFS= read -r ENGINE_VERSION; do
  prepareRelease "UnrealEngine" \
    "${ENGINE_VERSION}" \
    "${ENGINE_VERSION}-${GDK_VERSION}-rc" \
    "${ENGINE_VERSION}-release" \
    "$(buildkite-agent meta-data get UnrealEngine-pr-url)"
done <<< "${ENGINE_VERSIONS}"

echo "--- Writing metadata :pencil2:"
writeBuildkiteMetadata "./logs/bk-metadata"
