#!/usr/bin/env bash

# exit immediately on failure, or if an undefined variable is used
set -eu

REPO="${1}"
BRANCH_NAME="$(buildkite-agent meta-data get ${REPO}-release-branch)"

echo "steps:"
echo "  - trigger: ${REPO}-release-qa"
echo "    label: Release QA for ${REPO}"
echo "    build:"
echo "      branch: ${BRANCH_NAME}"
