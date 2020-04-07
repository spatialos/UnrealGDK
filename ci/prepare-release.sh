#!/usr/bin/env bash

### This script should only be run on Improbable's internal build machines.
### If you don't work at Improbable, this may be interesting as a guide to what software versions we use for our
### automation, but not much more than that.

set -e -u -o pipefail

if [[ -n "${DEBUG-}" ]]; then
  set -x
fi

if [[ -z "$BUILDKITE" ]]; then
  echo "This script is only to be run on Improbable CI."
  exit 1
fi

cd "$(dirname "$0")/../"

source ci/common-release.sh

REPO="${1}" # REPO = the first argument passed to the script when it was run.
RELEASE_VERSION="$(buildkite-agent meta-data get release-version)"

setupReleaseTool

mkdir -p ./logs

echo "--- Preparing ${REPO} @ ${RELEASE_VERSION} :package:"
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
