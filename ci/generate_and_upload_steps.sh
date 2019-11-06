#!/bin/bash
set -euo pipefail

buildkite-agent pipeline upload ci/stage_2_premerge.steps.yaml
