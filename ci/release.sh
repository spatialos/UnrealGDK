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

source ci/common.sh

REPO="${1}"
RELEASE_VERSION="$(buildkite-agent meta-data get release-version)"
PR_URL="$(buildkite-agent meta-data get ${REPO}-pr-url)"

setupReleaseTool

mkdir -p ./logs

echo "--- Releasing ${REPO} @ ${RELEASE_VERSION} :tada:"
docker run \
    -v "${SECRETS_DIR}":/var/ssh \
    -v "${SECRETS_DIR}":/var/github \
    -v "$(pwd)"/logs:/var/logs \
    local:gdk-release-tool \
        release "${RELEASE_VERSION}" \
            --github-key-file="/var/github/github_token" \
            --buildkite-metadata-path="/var/logs/bk-metadata" \
            --pull-request-url="${PR_URL}"

echo "--- Writing metadata :pencil2:"
writeBuildkiteMetadata "./logs/bk-metadata"
