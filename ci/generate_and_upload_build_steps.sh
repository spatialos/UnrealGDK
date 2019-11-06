#!/bin/bash
set -euo pipefail

# This script generates steps for each engine version listed in unreal-engine.version, and adds those to generated_base.steps.yaml
# The steps are based on the template in generated_steps.steps.yaml

if [ -z "${ENGINE_VERSION}" ]; then 
    echo "Generating build steps for each version listed in unreal-engine.version"  
    IFS=$'\n'
    for commit_hash in $(cat < ci/unreal-engine.version); do
        sed "s/INSERT_ENGINE_COMMIT_HASH/$commit_hash/g" ci/generated_steps.steps.yaml >> ci/generated_base.steps.yaml
    done
else
    echo "Generating steps for specified engine version: $ENGINE_VERSION" 
    sed "s/INSERT_ENGINE_COMMIT_HASH/$ENGINE_VERSION/g" ci/generated_steps.steps.yaml >> ci/generated_base.steps.yaml
fi;

buildkite-agent pipeline upload ci/generated_base.steps.yaml
