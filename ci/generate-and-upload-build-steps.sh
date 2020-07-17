#!/bin/bash
set -euo pipefail

upload_build_configuration_step() {
    export ENGINE_COMMIT_HASH="${1}"
    export BUILD_PLATFORM="${2}"
    export BUILD_TARGET="${3}"
    export BUILD_STATE="${4}"
    export TEST_CONFIG="${5:-default}"

    if [[ ${BUILD_PLATFORM} == "Mac" ]]; then
        export BUILD_COMMAND="./ci/setup-build-test-gdk.sh"
        REPLACE_STRING="s|BUILDKITE_AGENT_PLACEHOLDER|macos|g;"
    else
        export BUILD_COMMAND="powershell ./ci/setup-build-test-gdk.ps1"
        REPLACE_STRING="s|BUILDKITE_AGENT_PLACEHOLDER|windows|g;"
    fi

    sed "$REPLACE_STRING" "ci/gdk_build.template.steps.yaml" | buildkite-agent pipeline upload
}

generate_build_configuration_steps () {
    # See https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html for possible configurations 
    ENGINE_COMMIT_HASH="${1}"

    if [[ -z "${NIGHTLY_BUILD+x}" ]]; then
        export BK_MACHINE_TYPE="quad-high-cpu"
    else
        export BK_MACHINE_TYPE="single-high-cpu" # nightly builds run on smaller nodes
    fi

    if [[ -z "${MAC_BUILD:-}" ]]; then
        # if the BUILD_ALL_CONFIGURATIONS environment variable doesn't exist, then...
        if [[ -z "${BUILD_ALL_CONFIGURATIONS+x}" ]]; then
            echo "Building for subset of supported configurations. Generating the appropriate steps..."

            SLOW_NETWORKING_TESTS_LOCAL="${SLOW_NETWORKING_TESTS:-false}"
            # if the SLOW_NETWORKING_TESTS variable is not set or empty, look at whether this is a nightly build
            if [[ -z "${SLOW_NETWORKING_TESTS+x}" ]]; then
                if [[ "${NIGHTLY_BUILD:-false,,}" == "true" ]]; then
                    SLOW_NETWORKING_TESTS_LOCAL="true"
                fi
            fi

            export SLOW_NETWORKING_TESTS="${SLOW_NETWORKING_TESTS_LOCAL}"
            if [[ "${SLOW_NETWORKING_TESTS_LOCAL,,}" == "true" ]]; then
                # Start a build with native tests as a separate step
                upload_build_configuration_step "${ENGINE_COMMIT_HASH}" "Win64" "Editor" "Development" "Native"
            fi

            # Win64 Development Editor build configuration
            upload_build_configuration_step "${ENGINE_COMMIT_HASH}" "Win64" "Editor" "Development"

            # Linux Development NoEditor build configuration
            upload_build_configuration_step "${ENGINE_COMMIT_HASH}" "Linux" "" "Development"
        else
            echo "Building for all supported configurations. Generating the appropriate steps..."

            export BK_MACHINE_TYPE="single-high-cpu" # run the weekly with smaller nodes, since this is not time-critical

            # Editor builds (Test and Shipping build states do not exist for the Editor build target)
            for BUILD_STATE in "DebugGame" "Development"; do
                upload_build_configuration_step "${ENGINE_COMMIT_HASH}" "Win64" "Editor" "${BUILD_STATE}"
            done

            # Generate all possible builds for non-Editor build targets
            for BUILD_PLATFORM in "Win64" "Linux"; do
                for BUILD_TARGET in "" "Client" "Server"; do
                    for BUILD_STATE in "DebugGame" "Development" "Shipping"; do
                        upload_build_configuration_step "${ENGINE_COMMIT_HASH}" "${BUILD_PLATFORM}" "${BUILD_TARGET}" "${BUILD_STATE}"
                    done
                done
            done
        fi
    else
        if [[ -n "${SLOW_NETWORKING_TESTS:-}" ]]; then
            export SLOW_NETWORKING_TESTS
        fi

        if [[ -z "${BUILD_ALL_CONFIGURATIONS+x}" ]]; then
            # MacOS Development Editor build configuration
            upload_build_configuration_step "${ENGINE_COMMIT_HASH}" "Mac" "Editor" "Development"
        else
            # Editor builds (Test and Shipping build states do not exist for the Editor build target)
            for BUILD_STATE in "DebugGame" "Development"; do
                upload_build_configuration_step "${ENGINE_COMMIT_HASH}" "Mac" "Editor" "${BUILD_STATE}"
            done
        fi

    fi
}

# This script generates steps for each engine version listed in unreal-engine.version, 
# based on the gdk_build.template.steps.yaml template
if [[ -z "${ENGINE_VERSION+x}" ]]; then
    echo "Generating build steps for each engine version listed in unreal-engine.version"
    IFS=$'\n'
    for COMMIT_HASH in $(cat < ci/unreal-engine.version); do
        generate_build_configuration_steps "${COMMIT_HASH}"
    done
else
    echo "Generating steps for the specified engine version: ${ENGINE_VERSION}" 
    generate_build_configuration_steps "${ENGINE_VERSION}"
fi;
