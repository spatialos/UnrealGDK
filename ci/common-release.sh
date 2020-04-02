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
        --secret-name=unity-gdk/github-bot-personal-access-token \
        --field="token" \
        --write-to="${SECRETS_DIR}/github_token"

    imp-ci secrets read \
        --environment=production \
        --buildkite-org=improbable \
        --secret-type=ssh-key \
        --secret-name=unity-gdk/github-bot-ssh-key \
        --field="privateKey" \
        --write-to="${SECRETS_DIR}/id_rsa"

    docker build \
        --tag local:gdk-release-tool \
        --file ./ci/docker/release-tool.Dockerfile \
        .
}

function writeBuildkiteMetadata() {
    BK_METADATA_FILE="${1}"

    # If we want to upload any metadata elements. Do so here.
    if [[ -f "${BK_METADATA_FILE}" ]]; then
        while IFS= read -r line
        do
            KEY=$(echo "${line}" | cut -d "," -f 1)
            VALUE=$(echo "${line}" | cut -d "," -f 2-)
            buildkite-agent meta-data set "${KEY}" "${VALUE}"
        done < "${BK_METADATA_FILE}"
    fi
}
