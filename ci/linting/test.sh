#!/usr/bin/env bash

set -e -u -o pipefail

cd "$(dirname "$0")/../../"

source ci/pinned-tools.sh
source ci/profiling.sh

markStartOfBlock "$0"

markTestStarted "lint-code"
  "${IMP_LINT_BIN}" check \
    --linter cpp,json,git_merge_conflict \
    --exclude "build/" \
    --exclude ".spatialos/" \
    --exclude ".vs/" \
    --exclude "go/src/improbable.io/vendor" \
    --exclude "Source/SpatialGDK/Public/WorkerSdk"

  # go run "${GOPATH}/src/improbable.io/linter/main.go" check \
  #   "Source/SpatialOS" \
  #   "Source/Sdk" \
  #   "Plugins" \
markTestFinished "lint-code"

# markTestStarted "lint-docs"
#   go run "${GOPATH}/src/improbable.io/lint-docs/main.go" check \
#     "github_docs"
# markTestFinished "lint-docs"

markEndOfBlock "$0"
