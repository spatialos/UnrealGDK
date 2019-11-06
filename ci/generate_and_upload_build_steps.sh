#!/bin/bash
set -euo pipefail

# This script generates steps for each engine version listed in unreal-engine.version, and adds those to generated_base.steps.yaml
# The steps are based on the template in generated_steps.steps.yaml

IFS=$'\n'
for commit_hash in $(cat < ci/unreal-engine.version); do
    sed "s/INSERT_ENGINE_COMMIT_HASH/$commit_hash/g" ci/generated_steps.steps.yaml >> ci/generated_base.steps.yaml
done

buildkite-agent pipeline upload ci/generated_base.steps.yaml
