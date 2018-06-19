#!/usr/bin/env bash

set -e -u -o pipefail

cd "$(dirname "$0")/../"

HOOK_SOURCE="$(pwd)/ci/hooks/post-merge"
HOOK_TARGET="$(pwd)/.git/hooks/post-merge"

if [ ! -e "${HOOK_TARGET}" ]; then
    echo "Copying the post-merge hook ${HOOK_SOURCE} -> ${HOOK_TARGET}."
    cp "${HOOK_SOURCE}" "${HOOK_TARGET}"
else
    echo "A post-merge hook already exists at ${HOOK_TARGET}."
fi

HOOK_SOURCE="$(pwd)/ci/hooks/post-checkout"
HOOK_TARGET="$(pwd)/.git/hooks/post-checkout"

if [ ! -e "${HOOK_TARGET}" ]; then
    echo "Copying the post-checkout hook ${HOOK_SOURCE} -> ${HOOK_TARGET}."
    cp "${HOOK_SOURCE}" "${HOOK_TARGET}"
else
    echo "A post-checkout hook already exists at ${HOOK_TARGET}."
fi
