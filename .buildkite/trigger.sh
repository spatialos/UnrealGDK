#!/usr/bin/env bash
# https://brevi.link/shell-style
# https://explainshell.com
set -euo pipefail
if [[ -n "${DEBUG-}" ]]; then
  set -x
fi
cd "$(dirname "$0")/../"

# The step-definitions file is uploaded dynamically to preserve ability for historical builds
# vs changes in CI pipeline configuration.
buildkite-agent pipeline upload "$1"
