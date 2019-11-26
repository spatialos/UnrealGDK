#!/bin/bash
set -euo pipefail

upload_build_configuration_step() {
    cat "ci/gdk_build.template.steps.yaml" | \
    sed "s|ENGINE_COMMIT_HASH_PLACEHOLDER|$1|g" | \
    sed "s|BUILD_PLATFORM_PLACEHOLDER|$2|g" | \
    sed "s|BUILD_TARGET_PLACEHOLDER|$3|g" | \
    sed "s|BUILD_STATE_PLACEHOLDER|$4|g" | \
    buildkite-agent pipeline upload
}

generate_build_configuration_steps () {
    # See https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html for possible configurations 

    # if BUILD_ALL_CONFIGURATIONS environment variable exists AND is equal to "true", then...
    if [[ "${BUILD_ALL_CONFIGURATIONS+x}" = "x" ]] && [[ "$BUILD_ALL_CONFIGURATIONS" = "true" ]]; then
        echo "This is a nightly build. Generating the appropriate steps..."
        
        # Editor builds (Test and Shipping build states do not exist for the Editor build target)
        for build_platform in "Win64" "Linux"; do
            for build_state in "DebugGame" "Development"; do
                upload_build_configuration_step "$1" "$build_platform" "Editor" "$build_state"
            done
        done

        # NoEditor, Client and Server builds
        if [[ "$1" == *"4.22"* ]]; then
            # Prebuilt engines of native 4.22 and prior do not support Client and Server targets.
            # We use prebuilt engines in CI, but have manually added two Server configurations:
            for build_state in "Development" "Shipping"; do
                upload_build_configuration_step "$1" "Linux" "Server" "$build_state"
            done

            # NoEditor builds
            for build_platform in "Win64" "Linux"; do
                for build_state in "DebugGame" "Development" "Shipping" "Test"; do
                    upload_build_configuration_step "$1" "$build_platform" "" "$build_state"
                done
            done
        else {
            # Generate all possible builds for non-Editor build targets
            for build_platform in "Win64" "Linux"; do
                for build_target in "" "Client" "Server"; do
                    for build_state in "DebugGame" "Development" "Shipping" "Test"; do
                        upload_build_configuration_step "$1" "$build_platform" "$build_target" "$build_state"
                    done
                done
            done
        }
        fi
    else {
        echo "This is not a nightly build. Generating appropriate steps..."
        
        # Win64 Development Editor build configuration
        upload_build_configuration_step "$1" "Win64" "Editor" "Development"

        # Linux Development NoEditor build configuration
        upload_build_configuration_step "$1" "Linux" "" "Development"
    }
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
