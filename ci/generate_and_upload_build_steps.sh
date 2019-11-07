#!/bin/bash
set -euo pipefail

# This script generates steps for each engine version listed in unreal-engine.version, and adds those to generated_base.steps.yaml
# The steps are based on the template in generated_steps.steps.yaml

if [ -z "${ENGINE_VERSION}" ]; then 
    echo "Generating build steps for each engine version listed in unreal-engine.version"  
    IFS=$'\n'
    for commit_hash in $(cat < ci/unreal-engine.version); do
        sed "s/INSERT_ENGINE_COMMIT_HASH/$commit_hash/g" ci/gdk_build_template.steps.yaml | buildkite-agent pipeline upload
    done
else
    echo "Generating steps for the specified engine version: $ENGINE_VERSION" 
    sed "s/INSERT_ENGINE_COMMIT_HASH/$ENGINE_VERSION/g" ci/gdk_build_template.steps.yaml | buildkite-agent pipeline upload
fi;
