#!/usr/bin/env bash

# Use escaped characters to avoid an echo of the following line (due to
# set -x) triggering TeamCity - we only want the output of the echo to do so.

function markStartOfBlock {
  echo -e "\x23\x23teamcity[blockOpened name='$1']"
}

function markEndOfBlock {
  echo -e "\x23\x23teamcity[blockClosed name='$1']"
}
