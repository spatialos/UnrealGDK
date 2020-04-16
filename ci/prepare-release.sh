#!/usr/bin/env bash

### This script should only be run on Improbable's internal build machines.
### If you don't work at Improbable, this may be interesting as a guide to what software versions we use for our
### automation, but not much more than that.

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

# This assigns the first argument passed to this script to the variable REPO
REPO="${1}"
# This assigns the gdk-version key that was set in .buildkite\release.steps.yaml to the variable GDK-VERSION
GDK_VERSION="$(buildkite-agent meta-data get gdk-version)"
# This assigns the engine-version key that was set in .buildkite\release.steps.yaml to the variable ENGINE-VERSION
ENGINE_VERSIONS="$(buildkite-agent meta-data get engine-version)"
#Thie removes line breaks, replaces them with commas to form a list
ENGINE_VERSIONS="${ENGINE_VERSIONS//\\n/,}"

setupReleaseTool

mkdir -p ./logs

if [[ "${REPO}" == "UnrealEngine" ]]; then
echo "--- Preparing ${REPO} @ ${ENGINE_VERSIONS}, ${RELEASE_VERSION} :package:"
else
echo "--- Preparing ${REPO} @ ${RELEASE_VERSION} :package:"
fi

if [[ "${REPO}" != "UnrealGDK" ]]; then
	PIN_HASH="$(buildkite-agent meta-data get UnrealGDK-hash)"
	PIN_ARG="--update-pinned-gdk=${PIN_HASH}"
else
	PIN_ARG=""
fi

docker run \
    -v "${SECRETS_DIR}":/var/ssh \
    -v "${SECRETS_DIR}":/var/github \
    -v "$(pwd)"/logs:/var/logs \
    local:gdk-release-tool \
        prep "${RELEASE_VERSION}" \
        --git-repository-name="${REPO}" \
        --github-key-file="/var/github/github_token" \
        --buildkite-metadata-path="/var/logs/bk-metadata" ${PIN_ARG}

echo "--- Writing metadata :pencil2:"
writeBuildkiteMetadata "./logs/bk-metadata"
