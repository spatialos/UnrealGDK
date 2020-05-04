#!/usr/bin/env bash

set -e -u -o pipefail

if [[ -n "${DEBUG-}" ]]; then
  set -x
fi

USER_ID=${LOCAL_USER_ID:-999}

useradd --shell /bin/bash -u "${USER_ID}" -o -c "" -m user
export HOME=/home/user

# Change ownership of directories to the "user" user.
chown -R user:user "${HOME}"
chown -R user:user "$(pwd)"
chown -R user:user "/var/ssh"
chown -R user:user "/var/github"
chown -R user:user "/var/logs"
chown -R user:user "/root/.ssh"
cp -R /root/.ssh "${HOME}/.ssh"

gosu user dotnet ReleaseTool.dll "$@"