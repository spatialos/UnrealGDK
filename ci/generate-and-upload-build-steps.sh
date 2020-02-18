#!/bin/bash
set -euo pipefail

upload_build_configuration_step() {
    export ENGINE_COMMIT_HASH=$1
    export BUILD_PLATFORM=$2
    export BUILD_TARGET=$3
    export BUILD_STATE=$4
    buildkite-agent pipeline upload "ci/gdk_build.template.steps.yaml"
}

generate_build_configuration_steps () {
    # See https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html for possible configurations 
    ENGINE_COMMIT_HASH="${1}"

    # if the BUILD_ALL_CONFIGURATIONS environment variable doesn't exist, then...
    if [[ -z "${BUILD_ALL_CONFIGURATIONS+x}" ]]; then
        echo "Building for minimal subset of supported configurations. Generating the appropriate steps..."
        
        # Win64 Development Editor build configuration
        upload_build_configuration_step "${ENGINE_COMMIT_HASH}" "Win64" "Editor" "Development"

        # Linux Development NoEditor build configuration
        upload_build_configuration_step "${ENGINE_COMMIT_HASH}" "Linux" "" "Development"
    else
        echo "Building for all supported configurations. Generating appropriate steps..."
        
        # Editor builds (Test and Shipping build states do not exist for the Editor build target)
        for BUILD_STATE in "DebugGame" "Development"; do
             upload_build_configuration_step "${ENGINE_COMMIT_HASH}" "Win64" "Editor" "${BUILD_STATE}"
        done

        # NoEditor, Client and Server builds
        if [[ "${ENGINE_COMMIT_HASH}" == *"4.22"* ]]; then
            # Prebuilt engines of native 4.22 and prior do not support Client and Server targets.
            # We use prebuilt engines in CI, but have manually added two Server configurations:
            for BUILD_STATE in "Development" "Shipping"; do
                upload_build_configuration_step "${ENGINE_COMMIT_HASH}" "Linux" "Server" "${BUILD_STATE}"
            done

            # NoEditor builds
            for BUILD_PLATFORM in "Win64" "Linux"; do
                for BUILD_STATE in "DebugGame" "Development" "Shipping"; do
                    upload_build_configuration_step "${ENGINE_COMMIT_HASH}" "${BUILD_PLATFORM}" "" "${BUILD_STATE}"
                done
            done
        else
            # Generate all possible builds for non-Editor build targets
            for BUILD_PLATFORM in "Win64" "Linux"; do
                for BUILD_TARGET in "" "Client" "Server"; do
                    for BUILD_STATE in "DebugGame" "Development" "Shipping"; do
                        upload_build_configuration_step "${ENGINE_COMMIT_HASH}" "${BUILD_PLATFORM}" "${BUILD_TARGET}" "${BUILD_STATE}"
                    done
                done
            done
        fi
    fi;
}

# This script generates steps for each engine version listed in unreal-engine.version, 
# based on the gdk_build.template.steps.yaml template
if [[ -z "${ENGINE_VERSION}" ]]; then 
    echo "Generating build steps for each engine version listed in unreal-engine.version"  
    IFS=$'\n'
    for COMMIT_HASH in $(cat < ci/unreal-engine.version); do
        generate_build_configuration_steps "${COMMIT_HASH}"
    done
else
    echo "Generating steps for the specified engine version: ${ENGINE_VERSION}" 
    generate_build_configuration_steps "${ENGINE_VERSION}"
fi;
