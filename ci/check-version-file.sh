#!/bin/bash
set -euo pipefail

# This script enforces that certain 'protected' branches always stay on a pinned engine version, 
# so that each commit on those branches captures the specific engine version against which it should be built. 
# If a PR attempts to merge into a protected branch while using a non-pinned engine version, fail this step and thus block the merge.

protected_branches=(release preview)

is_protected=0
for item in ${protected_branches[@]}
do
    # If this commit is part of a PR merging into one of the protected branches, make sure we are on a pinned engine version.
    # IMPORTANT: For this to work, make sure that a new build is triggered in buildkite when a PR is opened. (This is a pipeline setting in the buildkite web UI)
    # If not, buildkite may re-use a build of this branch before the PR was created, in which case the merge target was not known, and this check will have passed.
    if [ "$BUILDKITE_PULL_REQUEST_BASE_BRANCH" == "$item" ]; then
        is_protected=1
    fi

    # Also check when we're on a protected branch itself, in case a non-pinned engine version somehow got into the branch.
    if [ "$BUILDKITE_BRANCH" == "$item" ]; then
        is_protected=1
    fi
done

if [ $is_protected -eq 1 ]; then
    engine_version=$(cat ci/unreal-engine.version)
    
    echo "Found engine version $engine_version"

    if [[ $engine_version == HEAD* ]]; then # version starts with "HEAD"
        error_msg="The merge target branch does not allow a floating (HEAD) engine version. Use a pinned version. (Of the form UnrealEngine-{commit hash})"
        
        echo $error_msg | buildkite-agent annotate --context "check-version-file" --style error

        printf '%s\n' "$error_msg" >&2
        exit 1
    fi
fi
