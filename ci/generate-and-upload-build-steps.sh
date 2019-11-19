#!/bin/bash
set -euo pipefail

# This script generates steps for each engine version listed in unreal-engine.version, based on the gdk_build.template.steps.yaml template
if [ -z "${ENGINE_VERSION}" ]; then 
    echo "Generating build steps for each engine version listed in unreal-engine.version"  
    IFS=$'\n'
    for commit_hash in $(cat < ci/unreal-engine.version); do
        sed "s|ENGINE_COMMIT_HASH_PLACEHOLDER|$commit_hash|g" ci/gdk_build.template.steps.yaml | buildkite-agent pipeline upload
    done
else
    echo "Generating steps for the specified engine version: $ENGINE_VERSION" 
    sed "s|ENGINE_COMMIT_HASH_PLACEHOLDER|$ENGINE_VERSION|g" ci/gdk_build.template.steps.yaml | buildkite-agent pipeline upload
fi;
