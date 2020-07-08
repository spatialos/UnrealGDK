function cleanUp() {
    rm -rf ${SECRETS_DIR}
}

function setupReleaseTool() {
    echo "--- Setting up release tool :gear:"
    # Create temporary directory for secrets and set a trap to cleanup on exit.
    export SECRETS_DIR=$(mktemp -d)
    trap cleanUp EXIT

    imp-ci secrets read \
        --environment=production \
        --buildkite-org=improbable \
        --secret-type=github-personal-access-token \
        --secret-name=gdk-for-unreal-bot-github-personal-access-token \
        --field="token" \
        --write-to="${SECRETS_DIR}/github_token"

    imp-ci secrets read \
        --environment=production \
        --buildkite-org=improbable \
        --secret-type=ssh-key \
        --secret-name=gdk-for-unreal-bot-ssh-key \
        --field="privateKey" \
        --write-to="${SECRETS_DIR}/id_rsa"

    docker build \
        --tag local:gdk-release-tool \
        --file ./ci/docker/release-tool.Dockerfile \
        .
}
