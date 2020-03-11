#!/usr/bin/env bash

UNREAL_PATH=${1}
TEST_REPO_BRANCH=${2}
TEST_REPO_URL=${3}
TEST_REPO_UPORJECT_PATH=${4}
TESTP_REPO_PATH=${5}
GDK_HOME=${6}
BUILD_PLATFORM=${7}
BUILD_STATE=${8}
BUILD_TARGET=${9}

# Clone the testing project

rm -rf ${TESTP_REPO_PATH}
echo "Downloading the testing project from ${TEST_REPO_URL}"
git clone -b "${TEST_REPO_BRANCH}" "${TEST_REPO_URL}" "${TESTP_REPO_PATH}" --depth 1
if [[ $? -ne 0 ]]; then
  echo "Failed to clone testing project from $test_repo_url."
  exit 1
fi

# The Plugin does not get recognised as an Engine plugin, because we are using a pre-built version of the engine
# copying the plugin into the project's folder bypasses the issue
mkdir -p "${TESTP_REPO_PATH}/Game/Plugins/UnrealGDK" 
ln -s "${GDK_HOME}" "${TESTP_REPO_PATH}/Game/Plugins/UnrealGDK"

# Disable tutorials, otherwise the closing of the window will crash the editor due to some graphic context reason
echo "\r\n[/Script/IntroTutorials.TutorialStateSettings]\r\nTutorialsProgress=(Tutorial=/Engine/Tutorial/Basics/LevelEditorAttract.LevelEditorAttract_C,CurrentStage=0,bUserDismissed=True)\r\n" >> "${UNREAL_PATH}/Engine/Config/BaseEditorSettings.ini"

echo "--- Generating project files"
"${UNREAL_PATH}/Engine/Build/BatchFiles/Mac/Build.sh" -projectfiles -project="${TEST_REPO_UPORJECT_PATH}" -game -engine -progress
if [[ $? -ne 0 ]]; then
  echo "Failed to generate files for the testing project."
  exit 1
fi

echo "--- Building project"

pushd "${UNREAL_PATH}"
  "${UNREAL_PATH}/Engine/Build/BatchFiles/Mac/XcodeBuild.sh" "${BUILD_TARGET}" "${BUILD_PLATFORM}" "${BUILD_STATE}" "${TEST_REPO_UPORJECT_PATH}"
popd

if [[ $? -ne 0 ]]; then
  echo "Failed to build testing project."
  exit 1
fi
