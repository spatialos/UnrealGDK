#!/usr/bin/env bash

set -e -u -o pipefail

cd "$(dirname "$0")/../../"

source ci/pinned-tools.sh
source ci/profiling.sh

markStartOfBlock "$0"

  markStartOfBlock "UnrealSdk source files"

  "${IMP_LINT_BIN}" fix  \
    --linter cpp,json,git_merge_conflict \
    --exclude "build/" \
    --exclude ".spatialos/" \
    --exclude ".vs/" \
    --exclude "Source/Programs/Improbable.Unreal.CodeGeneration.Example" \
    --exclude "go/src/improbable.io/vendor"

    # go run "${GOPATH}/src/improbable.io/linter/main.go" fix \
    #   "Source/SpatialOS" \
    #   "Source/Sdk" \
    #   "Plugins" \

 markEndOfBlock "UnrealSdk source files"

markEndOfBlock "$0"
