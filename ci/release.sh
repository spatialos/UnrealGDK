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
  local GITHUB_ORG="${6}"
  local ENGINE_VERSIONS="${7:-}"

  echo "--- Preparing ${REPO_NAME}: Cutting ${CANDIDATE_BRANCH} from ${SOURCE_BRANCH}, and creating a PR into ${RELEASE_BRANCH} :package:"

  docker run \
    -v "${BUILDKITE_ARGS[@]}" \
    -v "${SECRETS_DIR}":/var/ssh \
    -v "${SECRETS_DIR}":/var/github \
    -v "$(pwd)"/logs:/var/logs \
    local:gdk-release-tool \
        release "${GDK_VERSION}" \
        --source-branch="${SOURCE_BRANCH}" \
        --candidate-branch="${CANDIDATE_BRANCH}" \
        --release-branch="${RELEASE_BRANCH}" \
        --github-key-file="/var/github/github_token" \
        --pull-request-url="${PR_URL}" \
        --github-organization="${GITHUB_ORG}" \
        --engine-versions="${ENGINE_VERSIONS}"
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

# This assigns the gdk-version key that was set in .buildkite\release.steps.yaml to the variable GDK-VERSION
GDK_VERSION="$(buildkite-agent meta-data get gdk-version)"

# This assigns the engine-version key that was set in .buildkite\release.steps.yaml to the variable ENGINE-VERSION
ENGINE_VERSIONS="$(buildkite-agent meta-data get engine-versions)"

setupReleaseTool

mkdir -p ./logs
USER_ID=$(id -u)

# Run the C Sharp Release Tool for each candidate we want to release.

# The format is:
# 1. REPO_NAME
# 2. SOURCE_BRANCH
# 3. CANDIDATE_BRANCH
# 4. RELEASE_BRANCH
# 5. PR_URL
# 6. GITHUB_ORG
# 7 onwards. ENGINE_VERSIONS

# Release UnrealEngine must run before UnrealGDK so that the resulting commits can be included in that repo's unreal-engine.version
while IFS= read -r ENGINE_VERSION; do
  release "UnrealEngine" \
    "dry-run/${ENGINE_VERSION}" \
    "${ENGINE_VERSION}-${GDK_VERSION}-rc" \
    "dry-run/${ENGINE_VERSION}-release" \
    "$(buildkite-agent meta-data get UnrealEngine-pr-url)" \
    "improbableio" \
    "${ENGINE_VERSIONS}"
done <<< "${ENGINE_VERSIONS}"

release "UnrealGDK"               "dry-run/master" "${GDK_VERSION}-rc" "dry-run/release" "$(buildkite-agent meta-data get UnrealGDK-pr-url)"               "spatialos"  "${ENGINE_VERSIONS}"
release "UnrealGDKExampleProject" "dry-run/master" "${GDK_VERSION}-rc" "dry-run/release" "$(buildkite-agent meta-data get UnrealGDKExampleProject-pr-url)" "spatialos"
release "UnrealGDKTestGyms"       "dry-run/master" "${GDK_VERSION}-rc" "dry-run/release" "$(buildkite-agent meta-data get UnrealGDKTestGyms-pr-url)"       "spatialos"
release "UnrealGDKEngineNetTest"  "dry-run/master" "${GDK_VERSION}-rc" "dry-run/release" "$(buildkite-agent meta-data get UnrealGDKEngineNetTest-pr-url)"  "improbable"
release "TestGymBuildKite"        "dry-run/master" "${GDK_VERSION}-rc" "dry-run/release" "$(buildkite-agent meta-data get UnrealGDKEngineNetTest-pr-url)"  "improbable"
