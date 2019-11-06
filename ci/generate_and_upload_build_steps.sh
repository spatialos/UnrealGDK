#!/bin/bash
set -eu pipefail

ci/inserted_steps.yaml >> ci/stage_2_premerge.steps 
buildkite-agent pipeline upload ci/stage_2_premerge.steps.yaml
