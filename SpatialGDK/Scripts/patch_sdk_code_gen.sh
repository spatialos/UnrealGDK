#!/usr/bin/env bash

set -e -u -x -o pipefail

cd "$(dirname "$0")"

pushd "../Generated/UClasses"

find -name '*.h' | xargs sed -i 's/SPATIALOS_API/SPATIALGDK_API/'
find -name '*.h' -o -name '*.cpp' | xargs sed -i 's/SpatialOSViewTypes.h/SpatialGDKViewTypes.h/'
find -name '*.h' -o -name '*.cpp' | xargs sed -i 's/SpatialOSWorkerTypes.h/SpatialGDKWorkerTypes.h/'

popd