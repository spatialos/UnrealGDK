#!/usr/bin/env bash

set -e -u -o pipefail
if [[ -n "${DEBUG-}" ]]; then
    set -x
fi

pushd "$(dirname "$0")"
    UNREAL_PATH="${1?Please enter the path to the Unreal Engine.}"
    TEST_REPO_BRANCH="${2?Please enter the branch that you want to test.}"
    TEST_REPO_URL="${3?Please enter the URL for the git repo you want to test.}"
    TEST_REPO_UPROJECT_PATH="${4?Please enter the path to the uproject inside the test repo.}"
    TEST_REPO_PATH="${5?Please enter the path where the test repo should be cloned to.}"
    GDK_HOME="${6?Please enter the path to the GDK for Unreal repo.}"
    BUILD_PLATFORM="${7?Please enter the build platform for your Unreal build.}"
    BUILD_STATE="${8?Please enter the build state for your Unreal build.}"
    BUILD_TARGET="${9? Please enter the build target for your Unreal build.}"

    # Clone the testing project
    echo "Cloning the testing project from ${TEST_REPO_URL}"
    git clone \
        --branch "${TEST_REPO_BRANCH}" \
        "${TEST_REPO_URL}" \
        "${TEST_REPO_PATH}" \
        --single-branch

    # The Plugin does not get recognised as an Engine plugin, because we are using a pre-built version of the engine
    # copying the plugin into the project's folder bypasses the issue
    mkdir -p "${TEST_REPO_PATH}/Game/Plugins"
    cp -R "${GDK_HOME}" "${TEST_REPO_PATH}/Game/Plugins/UnrealGDK"

    # Disable tutorials, otherwise the closing of the window will crash the editor due to some graphic context reason
    echo "\r\n[/Script/IntroTutorials.TutorialStateSettings]\r\nTutorialsProgress=(Tutorial=/Engine/Tutorial/Basics/LevelEditorAttract.LevelEditorAttract_C,CurrentStage=0,bUserDismissed=True)\r\n" >> "${UNREAL_PATH}/Engine/Config/BaseEditorSettings.ini"
    pushd "${UNREAL_PATH}"
        echo "--- Generating project files"
        "Engine/Build/BatchFiles/Mac/Build.sh" \
            -projectfiles \
            -project="${TEST_REPO_UPROJECT_PATH}" \
            -game \
            -engine \
            -progress

        echo "--- Building project"
            "Engine/Build/BatchFiles/Mac/XcodeBuild.sh" \
            "${BUILD_TARGET}" \
            "${BUILD_PLATFORM}" \
            "${BUILD_STATE}" \
            "${TEST_REPO_UPROJECT_PATH}"
    popd
popd
