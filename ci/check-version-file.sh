#!/bin/bash
set -euo pipefail

protected_branches=(release preview)

is_protected=0
for item in ${protected_branches[@]}
do
        # If merging into a protected branch, do the check
    if [ "$BUILDKITE_PULL_REQUEST_BASE_BRANCH" == "$item" ]; then
        is_protected=1
    fi

        # Also check when we're on a protected branch itself, in case something got past us
        # This can happen if a PR was created after the last commit in the source branch had its build run
        # In this case, buildkite will use the test result from that build, but at that point there was no PR, 
        # so no protection was enforced. 
    if [ "$BUILDKITE_BRANCH" == "$item" ]; then
        is_protected=1
    fi
done

if [ $is_protected -eq 1 ]; then
    engine_version=$(cat ci/unreal-engine.version)
    
    echo "Found engine version $engine_version"

    if [[ $engine_version == HEAD* ]]; then # version starts with HEAD
        error_msg="The merge target branch does not allow a floating (HEAD) engine version. Use a pinned version. (Of the form UnrealEngine-{commit hash})"
        
        echo $error_msg | buildkite-agent annotate --context "check-version-file" --style error

        printf '%s\n' "$error_msg" >&2
        exit 1
    fi
fi
