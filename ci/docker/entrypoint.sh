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
chown -R user:user "$(pwd)/tools"

TEST_RESULTS_DIR="/var/logs/nunit"
gosu user mkdir -p "${TEST_RESULTS_DIR}"

echo "--- Build Tools.sln :construction:"
gosu user dotnet build tools/Tools.sln

echo "--- Test DocsLinter.csproj :link:"
gosu user dotnet test --logger:"nunit;LogFilePath=${TEST_RESULTS_DIR}/docslinter-test-results.xml" "tools/DocsLinter/DocsLinter.csproj"

echo "--- Test ReleaseTool.csproj :fork:"
gosu user dotnet test --logger:"nunit;LogFilePath=${TEST_RESULTS_DIR}/releasetool-test-results.xml" "tools/ReleaseTool.Tests/ReleaseTool.Tests.csproj"
