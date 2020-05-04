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
  
  echo "--- Preparing ${REPO_NAME}: Cutting ${CANDIDATE_BRANCH} from ${SOURCE_BRANCH}, and creating a PR into ${RELEASE_BRANCH} :package:"

  docker run \
    -v "${BUILDKITE_ARGS[@]}" \
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

# This BUILDKITE ARGS section is sourced from: improbable/nfr-benchmark-pipeline/blob/feature/nfr-framework/run.sh
declare -a BUILDKITE_ARGS=()

if [[ -n "${BUILDKITE:-}" ]]; then
    declare -a BUILDKITE_ARGS=(
    "-e=BUILDKITE=${BUILDKITE}"
    "-e=BUILD_EVENT_CACHE_ROOT_PATH=/build-event-data"
    "-e=BUILDKITE_AGENT_ACCESS_TOKEN=${BUILDKITE_AGENT_ACCESS_TOKEN}"
    "-e=BUILDKITE_AGENT_ENDPOINT=${BUILDKITE_AGENT_ENDPOINT}"
    "-e=BUILDKITE_AGENT_META_DATA_CAPABLE_OF_BUILDING=${BUILDKITE_AGENT_META_DATA_CAPABLE_OF_BUILDING}"
    "-e=BUILDKITE_AGENT_META_DATA_ENVIRONMENT=${BUILDKITE_AGENT_META_DATA_ENVIRONMENT}"
    "-e=BUILDKITE_AGENT_META_DATA_PERMISSION_SET=${BUILDKITE_AGENT_META_DATA_PERMISSION_SET}"
    "-e=BUILDKITE_AGENT_META_DATA_PLATFORM=${BUILDKITE_AGENT_META_DATA_PLATFORM}"
    "-e=BUILDKITE_AGENT_META_DATA_SCALER_VERSION=${BUILDKITE_AGENT_META_DATA_SCALER_VERSION}"
    "-e=BUILDKITE_AGENT_META_DATA_AGENT_COUNT=${BUILDKITE_AGENT_META_DATA_AGENT_COUNT}"
    "-e=BUILDKITE_AGENT_META_DATA_WORKING_HOURS_TIME_ZONE=${BUILDKITE_AGENT_META_DATA_WORKING_HOURS_TIME_ZONE}"
    "-e=BUILDKITE_AGENT_META_DATA_MACHINE_TYPE=${BUILDKITE_AGENT_META_DATA_MACHINE_TYPE}"
    "-e=BUILDKITE_AGENT_META_DATA_QUEUE=${BUILDKITE_AGENT_META_DATA_QUEUE}"
    "-e=BUILDKITE_TIMEOUT=${BUILDKITE_TIMEOUT}"
    "-e=BUILDKITE_ARTIFACT_UPLOAD_DESTINATION=${BUILDKITE_ARTIFACT_UPLOAD_DESTINATION}"
    "-e=BUILDKITE_BRANCH=${BUILDKITE_BRANCH}"
    "-e=BUILDKITE_BUILD_CREATOR_EMAIL=${BUILDKITE_BUILD_CREATOR_EMAIL}"
    "-e=BUILDKITE_BUILD_CREATOR=${BUILDKITE_BUILD_CREATOR}"
    "-e=BUILDKITE_BUILD_ID=${BUILDKITE_BUILD_ID}"
    "-e=BUILDKITE_BUILD_URL=${BUILDKITE_BUILD_URL}"
    "-e=BUILDKITE_COMMIT=${BUILDKITE_COMMIT}"
    "-e=BUILDKITE_JOB_ID=${BUILDKITE_JOB_ID}"
    "-e=BUILDKITE_LABEL=${BUILDKITE_LABEL}"
    "-e=BUILDKITE_MESSAGE=${BUILDKITE_MESSAGE}"
    "-e=BUILDKITE_ORGANIZATION_SLUG=${BUILDKITE_ORGANIZATION_SLUG}"
    "-e=BUILDKITE_PIPELINE_SLUG=${BUILDKITE_PIPELINE_SLUG}"
    "--volume=/usr/bin/buildkite-agent:/usr/bin/buildkite-agent"
    "--volume=/usr/local/bin/imp-tool-bootstrap:/usr/local/bin/imp-tool-bootstrap"
    )
fi

setupReleaseTool

mkdir -p ./logs
USER_ID=$(id -u)

# This assigns the gdk-version key that was set in .buildkite\release.steps.yaml to the variable GDK-VERSION
GDK_VERSION="$(buildkite-agent meta-data get gdk-version)"

# This assigns the engine-version key that was set in .buildkite\release.steps.yaml to the variable ENGINE-VERSION
ENGINE_VERSIONS="$(buildkite-agent meta-data get engine-versions)"

# Run the C Sharp Release Tool for each candidate we want to cut.
prepareRelease "UnrealGDK"                "dry-run/master" "${GDK_VERSION}-rc" "dry-run/release" "spatialos"
prepareRelease "UnrealGDKExampleProject"  "dry-run/master" "${GDK_VERSION}-rc" "dry-run/release" "spatialos"
prepareRelease "UnrealGDKTestGyms"        "dry-run/master" "${GDK_VERSION}-rc" "dry-run/release" "spatialos"
prepareRelease "UnrealGDKEngineNetTest"   "dry-run/master" "${GDK_VERSION}-rc" "dry-run/release" "improbable"

while IFS= read -r ENGINE_VERSION; do
  prepareRelease "UnrealEngine" \
    "dry-run/${ENGINE_VERSION}" \
    "${ENGINE_VERSION}-${GDK_VERSION}-rc" \
    "dry-run/${ENGINE_VERSION}-release" \
    "improbableio"
done <<< "${ENGINE_VERSIONS}"
