#!/usr/bin/env bash

### This script should only be run on Improbable's internal build machines.
### If you don't work at Improbable, this may be interesting as a guide to what software versions we use for our
### automation, but not much more than that.

set -e -u -o pipefail

function getSecrets() {
    if [[ ${BUILDKITE:-} ]]; then

        export SPATIAL_OAUTH_DIR=$(mktemp -d)
        local SPATIAL_OAUTH_FILE="${SPATIAL_OAUTH_DIR}/oauth2/oauth2_refresh_token"

        imp-ci secrets read \
            --environment="production" \
            --secret-type="spatialos-service-account" \
            --buildkite-org="improbable" \
            --secret-name="unity-gdk-toolbelt" \
            --field="token" \
            --write-to="${SPATIAL_OAUTH_FILE}"

        export CLOUDSMITH_AUTH_DIR=$(mktemp -d)
        local CLOUDSMITH_CONFIG_FILE="${CLOUDSMITH_AUTH_DIR}/credentials.ini"

        imp-ci secrets read \
            --environment="production" \
            --secret-type="cloudsmith-api-key" \
            --buildkite-org="improbable" \
            --secret-name="unity-gdk-cloudsmith" \
            --field="token" \
            --write-to="${CLOUDSMITH_CONFIG_FILE}"

        # We don't want to write the API key to the console, so we use sed to append the contents to the front of the file to
        # leave it in the format that cloudsmith expects. See https://github.com/cloudsmith-io/cloudsmith-cli#configuration
        sed -i '1s/^/[default]\napi_key=/' "${CLOUDSMITH_CONFIG_FILE}"

        trap deleteSecrets INT TERM EXIT
    else 
        # TODO: Support MacOS/Linux?
        export SPATIAL_OAUTH_DIR="${LOCALAPPDATA}\\.improbable"
        export CLOUDSMITH_AUTH_DIR="${APPDATA}\\cloudsmith"
    fi
}

function deleteSecrets() {
    rm -rf "${SPATIAL_OAUTH_DIR}"
    rm -rf "${CLOUDSMITH_AUTH_DIR}"
}

GDK_DIR="gdk-for-unity"

if [[ -n "${DEBUG-}" ]]; then
  set -x
fi

cd "$(dirname "$0")/../"

if [[ -d "${GDK_DIR}" ]]; then
    rm -rf "${GDK_DIR}"
fi

git clone \
    --single-branch \
    --branch master \
    git@github.com:spatialos/UnrealGDK.git \
    "${GDK_DIR}"

getSecrets

docker build \
    --tag local:gdk-publish-packages \
    --file ./ci/docker/publish-packages.Dockerfile \
    .

EXTRA_DOCKER_ARGS=""

if [[ -n "${DEBUG-}" ]]; then
    EXTRA_DOCKER_ARGS="${EXTRA_DOCKER_ARGS} --env DEBUG=\"${DEBUG}\""
fi

if [[ -n "${DRY_RUN-}" ]]; then
    EXTRA_DOCKER_ARGS="${EXTRA_DOCKER_ARGS} --env DRY_RUN=\"${DRY_RUN}\""
fi


docker run -it \
    --volume "${SPATIAL_OAUTH_DIR}":/var/spatial_oauth \
    --volume "${CLOUDSMITH_AUTH_DIR}":/var/cloudsmith_credentials \
    ${EXTRA_DOCKER_ARGS} \
    local:gdk-publish-packages
