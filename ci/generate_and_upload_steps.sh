#!/bin/bash
set -euo pipefail

buildkite-agent pipeline upload stage_2_premerge.steps.yaml
