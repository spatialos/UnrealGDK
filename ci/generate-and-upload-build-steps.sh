#!/bin/bash
set -euo pipefail

generate_build_configuration_steps () {
    # Always build Development Editor
    for build_platform in "Win64" "Linux"; do
        cat "ci/gdk_build.template.steps.yaml" | \
        sed "s|ENGINE_COMMIT_HASH_PLACEHOLDER|$1|g" | \
        sed "s|BUILD_PLATFORM_PLACEHOLDER|$build_platform|g" | \
        sed "s|BUILD_TARGET_SUFFIX_PLACEHOLDER|Editor|g" | \
        sed "s|BUILD_STATE_PLACEHOLDER|Development|g" | \
        buildkite-agent pipeline upload
    done

    # if NIGHTLY_BUILD exists AND is equal to "true", build game, server and client for additional configurations
    if [[ "${NIGHTLY_BUILD+x}" = "x" ]] && [[ "$NIGHTLY_BUILD" = "true" ]]; then
        echo "This is a nightly build. Generating the appropriate steps..."
        for build_platform in "Win64" "Linux"; do
            for build_target_suffix in "" "Server" "SimulatedPlayer"; do
                for build_state in "DebugGame" "Development" "Shipping" "Test"; do
                    cat "ci/gdk_build.template.steps.yaml" | \
                    sed "s|ENGINE_COMMIT_HASH_PLACEHOLDER|$1|g" | \
                    sed "s|BUILD_PLATFORM_PLACEHOLDER|$build_platform|g" | \
                    sed "s|BUILD_TARGET_SUFFIX_PLACEHOLDER|$build_target_suffix|g" | \
                    sed "s|BUILD_STATE_PLACEHOLDER|$build_state|g" | \
                    buildkite-agent pipeline upload
                done
            done
        done
    fi;
}

# This script generates steps for each engine version listed in unreal-engine.version, based on the gdk_build.template.steps.yaml template
if [[ -z ${ENGINE_VERSION+x} ]] || [[ -z "${ENGINE_VERSION}" ]]; then  # if ENGINE_VERSION doesn't exist OR is empty
    echo "Generating build steps for each engine version listed in unreal-engine.version"  
    IFS=$'\n'
    for commit_hash in $(cat < ci/unreal-engine.version); do
        generate_build_configuration_steps "$commit_hash"
    done
else
    echo "Generating steps for the specified engine version: $ENGINE_VERSION" 
    generate_build_configuration_steps "$ENGINE_VERSION"
fi;
