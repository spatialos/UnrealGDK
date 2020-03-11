#!/usr/bin/env bash

UNREAL_PATH=${1}
BUILD_HOME=${2}
TEST_PROJECT_NAME=${3}
RESULTS_NAME=${4}
TESTS_PATH=${5:-SpatialGDK}
RUN_WITH_SPATIAL=${6:-}


TEST_REPO_PATH="${BUILD_HOME}/${TEST_PROJECT_NAME}"
UPROJECT_PATH="${BUILD_HOME}/${TEST_PROJECT_NAME}/${TEST_REPO_RELATIVE_UPROJECT_PATH}"
LOG_FILE_PATH="ci/${TEST_PROJECT_NAME}/${RESULTS_NAME}/tests.log"
TEST_REPO_MAP="${TEST_PROJECT_NAME}/${RESULTS_NAME}"
REPORT_OUTPUT_PATH="ci/${TEST_REPO_MAP}"


if [[ -n "${RUN_WITH_SPATIAL}" ]]; then
	echo "Generating snapshot and schema for testing project"
	"${UNREAL_EDITOR_PATH}" "${UPROJECT_PATH}" -NoShaderCompile -nopause -nosplash -unattended -nullRHI -run=GenerateSchemaAndSnapshots -MapPaths="${TEST_REPO_MAP}"
	cp "${TEST_REPO_PATH}/spatial/snapshots/${TEST_REPO_MAP}.snapshot" "${TEST_REPO_PATH}/spatial/snapshots/default.snapshot" 
fi

mkdir ${REPORT_OUTPUT_PATH}

pushd "${UNREAL_PATH}"
	"${UNREAL_PATH}/Engine/Binaries/Mac/UE4Editor.app/Contents/MacOS/UE4Editor" \
		"$(pwd)/${UPROJECT_PATH}" \
		"${TEST_REPO_MAP}"  \
		-execCmds="Automation RunTests ${TESTS_PATH}; Quit" \
		-TestExit="Automation Test Queue Empty" \
		-ReportOutputPath="$(pwd)/${REPORT_OUTPUT_PATH}" \
		-ABSLOG="$(pwd)/${LOG_FILE_PATH}" \
		-nopause \
		-nosplash \
		-unattended \
		-nullRHI \
		-OverrideSpatialNetworking=${RUN_WITH_SPATIAL}
popd

# TODO report tests
#echo "--- report-tests"
#"${GDK_HOME}/ci/report-tests.sh" "ci/${TEST_PROJECT_NAME}/VanillaTestResults"