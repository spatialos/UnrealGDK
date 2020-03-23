#!/usr/bin/env bash

set -e -u -o pipefail
if [[ -n "${DEBUG-}" ]]; then
    set -x
fi

pushd "$(dirname "$0")"
    # Unreal path is the path to the Engine directory. No symlinking for mac, because they seem to cause issues during the build. This should ultimately resolve to "/Users/buildkite-agent/builds/<agent name>/improbable//UnrealEngine".
    UNREAL_PATH="${1:-"$(pwd)/../../UnrealEngine"}"
    # THE GCS bucket that stores the built out Unreal Engine we want to retrieve
    GCS_PUBLISH_BUCKET="${2:-io-internal-infra-unreal-artifacts-production/UnrealEngine}"
    GDK_HOME="$(pwd)/.."

    pushd "${GDK_HOME}"
        # Fetch the version of Unreal Engine we need
        pushd "ci"
            # Allow overriding the engine version if required
            if [[ -n "${ENGINE_COMMIT_HASH:-}" ]]; then
                VERSION_DESCRIPTION="${ENGINE_COMMIT_HASH}"
                echo "Using engine version defined by ENGINE_COMMIT_HASH: ${VERSION_DESCRIPTION}"
            else
                # Read Engine version from the file and trim any trailing white spaces and new lines.
                VERSION_DESCRIPTION=$(head -n 1 unreal-engine.version)
                echo "Using engine version found in unreal-engine.version file: ${VERSION_DESCRIPTION}"
            fi

            # Check if we are using a 'floating' engine version, meaning that we want to get the latest built version of the engine on some branch
            # This is specified by putting "HEAD name/of-a-branch" in the unreal-engine.version file
            # If so, retrieve the version of the latest build from GCS, and use that going forward.
            HEAD_VERSION_PREFIX="HEAD "
            if [[ "${VERSION_DESCRIPTION}" == ${HEAD_VERSION_PREFIX}* ]]; then
                VERSION_BRANCH=${VERSION_DESCRIPTION#"${HEAD_VERSION_PREFIX}"} # Remove the prefix to just get the branch name
                VERSION_BRANCH=$(echo ${VERSION_BRANCH} | tr "/" "_") # Replace / with _ since / is treated as the folder seperator in GCS

                # Download the head pointer file for the given branch, which contains the latest built version of the engine from that branch
                HEAD_POINTER_GCS_PATH="gs://${GCS_PUBLISH_BUCKET}/HEAD/mac-${VERSION_BRANCH}.version"
                UNREAL_VERSION=$(gsutil cp "${HEAD_POINTER_GCS_PATH}" -) # the '-' at the end instructs gsutil to download the file and output the contents to stdout
            else
                UNREAL_VERSION="Mac-${VERSION_DESCRIPTION}"
            fi
        popd

        echo "--- download-unreal-engine"
        ENGINE_GCS_PATH="gs://${GCS_PUBLISH_BUCKET}/${UNREAL_VERSION}.zip"
        echo "Downloading Unreal Engine artifacts version ${UNREAL_VERSION} from ${ENGINE_GCS_PATH}"
        gsutil cp -n "${ENGINE_GCS_PATH}" "${UNREAL_VERSION}".zip
        7z x "${UNREAL_VERSION}".zip -o${UNREAL_PATH} -aos
    popd
popd
