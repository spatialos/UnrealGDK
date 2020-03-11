#!/usr/bin/env bash

UNREAL_PATH="${1:-$(pwd)/../UnrealEngine}"
BUILD_PROJECT=${2:-NetworkTestProject}

PROJECT_ABSOLUTE_PATH="$(pwd)/../${BUILD_PROJECT}"
GDK_IN_TEST_REPO="${PROJECT_ABSOLUTE_PATH}/Game/Plugins/UnrealGDK"

# Workaround for UNR-2156 and UNR-2076, where spatiald / runtime processes sometimes never close, or where runtimes are orphaned
# Clean up any spatiald and java (i.e. runtime) processes that may not have been shut down
#spatial service stop
#pkill -9 -f java

#rm -f ${UNREAL_PATH}
#rm -f ${GDK_IN_TEST_REPO}
#rm -rf ${PROJECT_ABSOLUTE_PATH}
