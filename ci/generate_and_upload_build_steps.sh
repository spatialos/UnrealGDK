#!/bin/bash
set -euo pipefail

cat ci/inserted_steps.yaml >> ci/stage_2_premerge.steps.yaml

buildkite-agent artifact upload ci/stage_2_premerge.steps.yaml

buildkite-agent pipeline upload ci/stage_2_premerge.steps.yaml
