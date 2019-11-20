#!/bin/bash
set -euo pipefail

generate_build_configuration_steps () {
    if [ "${NIGHTLY_BUILD+x}" == "x" ] && ["$NIGHTLY_BUILD" == "true"]; then 
        echo "This is a nightly build. Generating the appropriate steps..."
        for build_target_suffix in "" "Editor" "Server" "SimulatedPlayer"; do
            for build_state in "DebugGame" "Development" "Shipping" "Test"; do
                cat "ci/gdk_build.template.steps.yaml" | \
                sed "s|ENGINE_COMMIT_HASH_PLACEHOLDER|$1|g" | \
                sed "s|BUILD_TARGET_SUFFIX_PLACEHOLDER|$build_target_suffix|g" | \
                sed "s|BUILD_STATE_PLACEHOLDER|$build_state|g" | \
                buildkite-agent pipeline upload
            done
        done
    else
        echo "This is not a nightly build. Generating the appropriate steps..."
        cat "ci/gdk_build.template.steps.yaml" | \
        sed "s|ENGINE_COMMIT_HASH_PLACEHOLDER|$1|g" | \
        sed "s|BUILD_TARGET_SUFFIX_PLACEHOLDER|Editor|g" | \
        sed "s|BUILD_STATE_PLACEHOLDER|Development|g" | \
        buildkite-agent pipeline upload
    fi;
}

# This script generates steps for each engine version listed in unreal-engine.version, based on the gdk_build.template.steps.yaml template
if [ "${ENGINE_VERSION+x}" == "x" ] && [ "$ENGINE_VERSION" == "true" ]; then 
    echo "Generating build steps for each engine version listed in unreal-engine.version"  
    IFS=$'\n'
    for commit_hash in $(cat < ci/unreal-engine.version); do
        generate_build_configuration_steps "$commit_hash"
    done
else
    echo "Generating steps for the specified engine version: $ENGINE_VERSION" 
    generate_build_configuration_steps "$ENGINE_VERSION"
fi;
