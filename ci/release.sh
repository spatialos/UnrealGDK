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
  local RELEASE_ENGINE_BRANCHES_LOCAL_VAR="$(echo ${RELEASE_ENGINE_BRANCHES[*]// })"

  echo "--- Releasing ${REPO_NAME}: Merging ${CANDIDATE_BRANCH} into ${RELEASE_BRANCH} :package:"

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
        --pull-request-url="\"${PR_URL}\"" \
        --git-repository-name="${REPO_NAME}" \
        --github-organization="${GITHUB_ORG}" \
        --engine-versions="\"${RELEASE_ENGINE_BRANCHES_LOCAL_VAR}\""
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

# This assigns the (potential) dry-run prefix to this variable if we are doing a dry-run
DRY_RUN_PREFIX=$(getDryrunBranchPrefix)

# This assigns the engine-version key that was set in .buildkite\release.steps.yaml to the variable ENGINE-VERSION
ENGINE_VERSIONS=($(buildkite-agent meta-data get engine-source-branches))

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

# Release UnrealEngine must run before UnrealGDK so that the resulting commits can be included in that repo's unreal-engine.version.
# We go over the array in reverse order here, just to release the least relevant engine version first, so the most relevant one will
# end up on top of the releases page.
RELEASE_ENGINE_BRANCHES=()
for (( idx=${#ENGINE_VERSIONS[@]}-1 ; idx>=0 ; idx-- )) ; do
    ENGINE_VERSION=${ENGINE_VERSIONS[idx]}
    RELEASE_BRANCH_NAME="${DRY_RUN_PREFIX}${ENGINE_VERSION}-release"
    release "UnrealEngine" \
    "${ENGINE_VERSION}" \
    "${ENGINE_VERSION}-${GDK_VERSION}-rc" \
    "${RELEASE_BRANCH_NAME}" \
    "$(buildkite-agent meta-data get UnrealEngine-${ENGINE_VERSION}-pr-url)" \
    "improbableio"
    RELEASE_ENGINE_BRANCHES+=("${RELEASE_BRANCH_NAME}")
done

release "UnrealGDK"               "$(buildkite-agent meta-data get gdk-source-branch)" "${GDK_VERSION}-rc" "${DRY_RUN_PREFIX}release" "$(buildkite-agent meta-data get UnrealGDK-$(buildkite-agent meta-data get gdk-source-branch)-pr-url)"               "spatialos"
release "UnrealGDKExampleProject" "$(buildkite-agent meta-data get gdk-source-branch)" "${GDK_VERSION}-rc" "${DRY_RUN_PREFIX}release" "$(buildkite-agent meta-data get UnrealGDKExampleProject-$(buildkite-agent meta-data get gdk-source-branch)-pr-url)" "spatialos"
release "UnrealGDKTestGyms"       "$(buildkite-agent meta-data get gdk-source-branch)" "${GDK_VERSION}-rc" "${DRY_RUN_PREFIX}release" "$(buildkite-agent meta-data get UnrealGDKTestGyms-$(buildkite-agent meta-data get gdk-source-branch)-pr-url)"       "spatialos"
release "UnrealGDKEngineNetTest"  "$(buildkite-agent meta-data get gdk-source-branch)" "${GDK_VERSION}-rc" "${DRY_RUN_PREFIX}release" "$(buildkite-agent meta-data get UnrealGDKEngineNetTest-$(buildkite-agent meta-data get gdk-source-branch)-pr-url)"  "improbable"
release "TestGymBuildKite"        "$(buildkite-agent meta-data get gdk-source-branch)" "${GDK_VERSION}-rc" "${GDK_VERSION}" "$(buildkite-agent meta-data get TestGymBuildKite-$(buildkite-agent meta-data get gdk-source-branch)-pr-url)"                  "improbable"
