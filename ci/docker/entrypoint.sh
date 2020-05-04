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

gosu git config --global user.name "UnrealGDK Bot"
gosu git config --global user.email "gdk-for-unreal-bot@improbable.io"
gosu git config --global core.sshCommand "ssh -i /var/ssh/id_rsa"

mkdir -p /${HOME}/.ssh
    touch /${HOME}/.ssh/known_hosts
    ssh-keyscan github.com >> /${HOME}/.ssh/known_hosts

gosu user dotnet ReleaseTool.dll "$@"