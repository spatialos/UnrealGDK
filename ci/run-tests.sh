#!/usr/bin/env bash

set -e -u -o pipefail
if [[ -n "${DEBUG-}" ]]; then
    set -x
fi

pushd "$(dirname "$0")"
    UNREAL_PATH="${1?Please enter the path to the Unreal Engine.}"
    TEST_PROJECT_NAME="${2?Please enter the name of the test project.}"
    UPROJECT_PATH="${3?Please enter the absolute path to the uproject path.}"
    RESULTS_NAME="${4?Please enter the name of the results folder.}"
    TEST_REPO_MAP="${5?Please specify which map to test.}"
    TESTS_PATH="${6:-SpatialGDK}"
    RUN_WITH_SPATIAL="${7:-}"

    GDK_HOME="$(pwd)/.."
    BUILD_HOME="$(pwd)/../.."
    TEST_REPO_PATH="${BUILD_HOME}/${TEST_PROJECT_NAME}"
    REPORT_OUTPUT_PATH="${GDK_HOME}/ci/${RESULTS_NAME}"
    LOG_FILE_PATH="${REPORT_OUTPUT_PATH}/tests.log"

    pushd "${UNREAL_PATH}"
        UNREAL_EDITOR_PATH="Engine/Binaries/Mac/UE4Editor.app/Contents/MacOS/UE4Editor"
        if [[ -n "${RUN_WITH_SPATIAL}" ]]; then
            echo "Generating snapshot and schema for testing project"
            "${UNREAL_EDITOR_PATH}" \
                "${UPROJECT_PATH}" \
                -SkipShaderCompile \
                -nopause \
                -nosplash \
                -unattended \
                -nullRHI \
                -run=CookAndGenerateSchema \
                -map="${TEST_REPO_MAP}" \
                -targetplatform=MacNoEditor \
                -cooksinglepackage
                
            "${UNREAL_EDITOR_PATH}" \
                "${UPROJECT_PATH}" \
                -NoShaderCompile \
                -nopause \
                -nosplash \
                -unattended \
                -nullRHI \
                -run=GenerateSnapshot \
                -MapPaths="${TEST_REPO_MAP}"

            cp "${TEST_REPO_PATH}/spatial/snapshots/${TEST_REPO_MAP}.snapshot" "${TEST_REPO_PATH}/spatial/snapshots/default.snapshot"
        fi

        mkdir "${REPORT_OUTPUT_PATH}"
        "${UNREAL_EDITOR_PATH}" \
            "${UPROJECT_PATH}" \
            "${TEST_REPO_MAP}"  \
            -execCmds="Automation RunTests ${TESTS_PATH}; Quit" \
            -TestExit="Automation Test Queue Empty" \
            -ReportOutputPath="${REPORT_OUTPUT_PATH}" \
            -ABSLOG="${LOG_FILE_PATH}" \
            -nopause \
            -nosplash \
            -unattended \
            -nullRHI \
            -OverrideSpatialNetworking="${RUN_WITH_SPATIAL}"
    popd

    # TODO: UNR-3167 - report tests
popd
