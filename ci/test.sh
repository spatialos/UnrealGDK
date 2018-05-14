#!/usr/bin/env bash

set -e -u -x -o pipefail

cd "$(dirname "$0")/../"

source ci/pinned-tools.sh
source ci/profiling.sh

markStartOfBlock "$0"

#####
# Setup variables.
#####
markStartOfBlock "Setup variables"

LOG_DIR="$(pwd)/build/logs/unreal_sdk"
CODE_GENERATION_TEST_BINARY="packages/NUnit.ConsoleRunner.3.8.0/tools/nunit3-console.exe"
CODE_GENERATION_TEST_ASSEMBLIES="Source/Programs/Improbable.Unreal.CodeGeneration.Test/bin/Release/Improbable.Unreal.CodeGeneration.Test.dll"
CODE_GENERATION_TEAMCITY_OPTION=""
if isTeamCity; then
  CODE_GENERATION_TEAMCITY_OPTION="--teamcity"
fi

UNREAL_GDK_TEST_DIR="$(pwd)/tests/unreal_gdk/workers/unreal"
UNREAL_GDK_TEST_PROJECT="${UNREAL_GDK_TEST_DIR}/Game/Sdk.uproject"
PROJECT_LOGFILE="${LOG_DIR}/AutomationTests.log"

markEndOfBlock "Setup variables"

#####
# Cleanup folders.
#####
markStartOfBlock "Cleanup folders"

# Clean up imported DLLS, etc.
rm -rf "${LOG_DIR}"

markEndOfBlock "Cleanup folders"

#####
# Create folders.
#####

markStartOfBlock "Create folders"

mkdir -p "${LOG_DIR}"

markEndOfBlock "Create folders"

#####
# Go tests.
#####
markStartOfBlock "Go tests"

if isTeamCity; then
  go test -v $(go list improbable.io/... | grep -v "vendor") | go run "${GOPATH}/src/improbable.io/vendor/github.com/2tvenom/go-test-teamcity/main.go"
else
  go test -v $(go list improbable.io/... | grep -v "vendor")
fi

markEndOfBlock "Go tests"

#####
# CodeGeneration tests.
#####
markStartOfBlock "CodeGeneration tests"

"${CODE_GENERATION_TEST_BINARY}" "${CODE_GENERATION_TEST_ASSEMBLIES}" \
  --work="Source/Programs/Improbable.Unreal.CodeGeneration.Test" \
  --result="${LOG_DIR}/CodeGenerationTestResults.xml" \
  ${CODE_GENERATION_TEAMCITY_OPTION}


markEndOfBlock "CodeGeneration tests"

#####
# Install the GDK
#####
markStartOfBlock "Install the GDK in the test project"

./setup.sh tests/unreal_gdk/

markEndOfBlock "Install the GDK in the test project"

#####
# Generate code.
#####

markStartOfBlock "Generate code"

pushd "${UNREAL_GDK_TEST_DIR}"

  spatial codegen UnrealClient --log_level=debug

#   # *********
#   # Giant hack to inject our mocked worker types into the test application
#   # *********
#   sed -i -e 's/SpatialOS\/Generated\/UClasses/SpatialOS\/Generated\/UClasses\",\n\"SpatialOS\/Mock/g' "${UNREAL_GDK_TEST_DIR}/Game/Source/SpatialOS/SpatialOS.Build.cs"
#   echo '#include "SpatialOSMockViewTypes.h"' > "${UNREAL_GDK_TEST_DIR}/Game/Source/SpatialOS/Public/SpatialOSViewTypes.h"
#   echo '#include "SpatialOSMockWorkerTypes.h"' > "${UNREAL_GDK_TEST_DIR}/Game/Source/SpatialOS/Public/SpatialOSWorkerTypes.h"

popd

markEndOfBlock "Generate code"

# #####
# # Ensure we compile with the previous version of Unreal.
# #####
# markStartOfBlock "Compile with ${PREVIOUS_UNREAL_VERSION}"

# # Reset intermediate files to avoid UBT-related errors that occur when switching between engine versions.
# rm -rf "${UNREAL_GDK_TEST_DIR}/Game/Intermediate/Build"

# # Build against the previously supported version of Unreal, but don't run tests as Unreal assets can be incompatible between versions.
# "${PREVIOUS_UNREAL_HOME}/Engine/Build/BatchFiles/Build.bat" SdkEditor Win64 Development "${UNREAL_GDK_TEST_PROJECT}"

# # Skip Linux builds as we do not have capacity in our CI for building without Incredibuild when targeting linux.
# # if isTeamCity; then
# #  "${PREVIOUS_UNREAL_HOME}\Engine\Build\BatchFiles\Build.bat" SdkServer Linux Development "${UNREAL_GDK_TEST_PROJECT}"
# # fi

# markEndOfBlock "Compile with ${PREVIOUS_UNREAL_VERSION}"

#####
# Build the current version of Unreal.
#####
markStartOfBlock "Compile with ${UNREAL_VERSION}"

# Reset intermediate files to avoid UBT-related errors that occur when switching between engine versions.
rm -rf "${UNREAL_GDK_TEST_DIR}/Game/Intermediate/Build"
"${UNREAL_HOME}/Engine/Build/BatchFiles/Build.bat" SdkEditor Win64 Development "${UNREAL_GDK_TEST_PROJECT}"

# Skip Linux builds as we do not have capacity in our CI for building without incredibuild when targeting linux.
# if isTeamCity; then
#  "${UNREAL_HOME}\Engine\Build\BatchFiles\Build.bat" SdkServer Linux Development "${UNREAL_GDK_TEST_PROJECT}"
# fi

markEndOfBlock "Compile with ${UNREAL_VERSION}"

# #####
# # Test current version.
# #####
# markStartOfBlock "Run Unreal tests (Unreal ${UNREAL_VERSION})"

# "${UNREAL_HOME}\Engine\Binaries\Win64\UE4Editor-Cmd.exe" \
#   "${UNREAL_GDK_TEST_PROJECT}" \
#   -server \
#   -stdout \
#   -unattended \
#   -buildmachine \
#   -execcmds="automation list;runtests SpatialOS;quit" \
#   -abslog="${PROJECT_LOGFILE}" \
#   -nullrhi \
#   -CrashForUAT

# markEndOfBlock "Run Unreal tests (Unreal ${UNREAL_VERSION})"

markEndOfBlock "$0"
