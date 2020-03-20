#!/usr/bin/env bash

set -e -u -o pipefail
if [[ -n "${DEBUG-}" ]]; then
    set -x
fi

pushd "$(dirname "$0")"
    UNREAL_PATH="${1?Please enter the path to the Unreal Engine.}"
    BUILD_HOME="${2?Please enter the path to the parent folder from the GDK repo.}"
    TEST_PROJECT_NAME="${3?Please enter the name of the test project.}"
    RESULTS_NAME="${4?Please enter the name of the results folder.}"
    TESTS_PATH="${5:-SpatialGDK}"
    RUN_WITH_SPATIAL="${6:-}"
    GDK_HOME="$(pwd)/.."

    TEST_REPO_PATH="${BUILD_HOME}/${TEST_PROJECT_NAME}"
    UPROJECT_PATH="${BUILD_HOME}/${TEST_PROJECT_NAME}/${TEST_REPO_RELATIVE_UPROJECT_PATH}"
    LOG_FILE_PATH="ci/${TEST_PROJECT_NAME}/${RESULTS_NAME}/tests.log"
    TEST_REPO_MAP="${TEST_PROJECT_NAME}/${RESULTS_NAME}"
    REPORT_OUTPUT_PATH="ci/${TEST_REPO_MAP}"

    if [[ -n "${RUN_WITH_SPATIAL}" ]]; then
        echo "Generating snapshot and schema for testing project"
        "${UNREAL_EDITOR_PATH}" \
            "${UPROJECT_PATH}" \
            -NoShaderCompile \
            -nopause \
            -nosplash \
            -unattended \
            -nullRHI \
            -run=GenerateSchemaAndSnapshots \
            -MapPaths="${TEST_REPO_MAP}"

        cp "${TEST_REPO_PATH}/spatial/snapshots/${TEST_REPO_MAP}.snapshot" "${TEST_REPO_PATH}/spatial/snapshots/default.snapshot" 
    fi

    mkdir ${REPORT_OUTPUT_PATH}

    pushd "${UNREAL_PATH}"
        "${UNREAL_PATH}/Engine/Binaries/Mac/UE4Editor.app/Contents/MacOS/UE4Editor" \
            "${GDK_HOME}/../${UPROJECT_PATH}" \
            "${TEST_REPO_MAP}"  \
            -execCmds="Automation RunTests ${TESTS_PATH}; Quit" \
            -TestExit="Automation Test Queue Empty" \
            -ReportOutputPath="${GDK_HOME}/${REPORT_OUTPUT_PATH}" \
            -ABSLOG="${GDK_HOME}/${LOG_FILE_PATH}" \
            -nopause \
            -nosplash \
            -unattended \
            -nullRHI \
            -OverrideSpatialNetworking=${RUN_WITH_SPATIAL}
    popd

    # TODO report tests
popd
