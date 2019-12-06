#!/bin/bash
set -euo pipefail

# This script enforces that some engine version is specified and that certain 'protected' branches always stay on a pinned engine versions, 
# so that each commit on those branches captures the specific engine version against which it should be built. 
# If a PR attempts to merge into a protected branch while using non-pinned engine versions, fail this step and thus block the merge.

# Ensure that at least one engine version is listed
if [$(cat ci/unreal-engine.version) = ""]; then
    ERROR_MSG="No version has been listed in the unreal-engine.version file."
            
    echo "${ERROR_MSG}" | buildkite-agent annotate --context "check-version-file" --style error

    printf '%s\n' "${ERROR_MSG}" >&2
    exit 1
fi;

# Enforce pinned engine versions on the following branches
protected_branches=(release preview)

IS_PROTECTED=0
for ITEM in ${protected_branches[@]}
do
    # If this commit is part of a PR merging into one of the protected branches, make sure we are on a pinned engine version.
    # IMPORTANT: For this to work, make sure that a new build is triggered in buildkite when a PR is opened. (This is a pipeline setting in the buildkite web UI)
    # If not, buildkite may re-use a build of this branch before the PR was created, in which case the merge target was not known, and this check will have passed.
    if [[ "${BUILDKITE_PULL_REQUEST_BASE_BRANCH}" == "${ITEM}" ]]; then
        IS_PROTECTED=1
    fi

    # Also check when we're on a protected branch itself, in case a non-pinned engine version somehow got into the branch.
    if [[ "${BUILDKITE_BRANCH}" == "${ITEM}" ]]; then
        IS_PROTECTED=1
    fi
done

if [[ $IS_PROTECTED -eq 1 ]]; then
    # Ensure that every listed engine version is pinned
    IFS=$'\n'
    for ENGINE_VERSION in $(cat < ci/unreal-engine.version); do
        echo "Found engine version ${ENGINE_VERSION}"

        if [[ $ENGINE_VERSION == HEAD* ]]; then # version starts with "HEAD"
            ERROR_MSG="The merge target branch does not allow floating (HEAD) engine versions. Use pinned versions. (Of the form UnrealEngine-{commit hash})"
            
            echo $ERROR_MSG | buildkite-agent annotate --context "check-version-file" --style error

            printf '%s\n' "${ERROR_MSG}" >&2
            exit 1
        fi
    done
fi
