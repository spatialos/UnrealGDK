#!/bin/bash
set -euo pipefail

BUILDKITE_PULL_REQUEST_BASE_BRANCH="release"

protected_branches=(release preview)

is_protected=0
for item in ${protected_branches[@]}
do
    if [ $BUILDKITE_PULL_REQUEST_BASE_BRANCH == $item ]; then
        is_protected=1
    fi
done

if [ $is_protected -eq 1 ]; then
    engine_version=$(cat ci/unreal-engine.version)
    echo -n $engine_version
    if [[ $engine_version == HEAD* ]]; then # version starts with HEAD
        error_msg="The merge target branch does not allow a floating (HEAD) engine version. Use a pinned version. (Of the form UnrealEngine-{commit hash}"
        
        echo $error_msg | buildkite-agent annotate --context "check-version-file" --style error

        printf '%s\n' $error_msg >&2
        exit 1
    fi
fi

# Expects $gdk_home
# param(
#     $pinned_branches = @("master", "release", "feature/ci-use-latest-engine-fake-master")
# )

# pushd "$($gdk_home)\ci"
#     $branch_name = (Get-Item -Path env:BUILDKITE_BRANCH).Value
#     if ($pinned_branches.Contains($branch_name)) {
#         $unreal_version = Get-Content -Path "unreal-engine.version" -Raw

#         if ($unreal_version.StartsWith("HEAD ")) {
#             $error_msg = "Using a HEAD engine version in unreal-engine.version! The branches $($pinned_branches -join ", ") are required to have a pinned engine version. (I.e. of the form UnrealEngine-{commit hash})"
#             echo $error_msg | buildkite-agent annotate --context "engine-version-file" --style error
#             Throw $error_msg
#         }
#     }
# popd