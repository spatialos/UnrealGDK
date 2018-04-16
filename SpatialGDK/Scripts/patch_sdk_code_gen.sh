#!/usr/bin/env bash

set -e -u -x -o pipefail

cd "$(dirname "$0")"

pushd "../Generated/UClasses"

find -name '*.h' | xargs sed -i 's/SPATIALOS_API/SPATIALGDK_API/'

popd