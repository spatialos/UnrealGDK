#!/usr/bin/env bash

set -e -u -x -o pipefail

cd "$(dirname "$0")/../"

ci/build.sh "$@"
ci/test.sh "$@"
# Currently we do not perform the publish operation
# This will be revisited when we get closer to release.
