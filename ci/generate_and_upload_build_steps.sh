#!/bin/bash
set -euo pipefail

IFS=$'\n'
for commit_hash in $(cat < ci/unreal-engine.version); do
    echo "hash: $commit_hash"
    echo "s/INSERT_ENGINE_COMMIT_HASH/$commit_hash/g"
done

for commit_hash in $(cat < ci/unreal-engine.version); do
    sed "s/INSERT_ENGINE_COMMIT_HASH/$commit_hash/g" ci/generated_steps.steps.yaml >> ci/generated_base.steps.yaml
done

buildkite-agent pipeline upload ci/generated_base.steps.yaml
