FROM python:3

# Install: 
#   - cloudsmith-cli            for publishing the packages.
#   - jq                        for parsing JSON (used in ./init.sh)
#   - gosu                      for handling issues with spatial writing files which buildkite-agent cannot delete.
#   - npm                       for packing up our packages
RUN pip install cloudsmith-cli \
    && apt-get update \
    && apt-get install -y jq \
    && curl -LSs -o /usr/local/bin/gosu -SL "https://github.com/tianon/gosu/releases/download/1.4/gosu-$(dpkg --print-architecture)" \
    && chmod +x /usr/local/bin/gosu \
    && curl -sL https://deb.nodesource.com/setup_9.x | bash - \
    && apt-get install -y nodejs \
    && curl -L https://www.npmjs.com/install.sh | sh \
    && apt-get clean

# Download spatial CLI into container
ARG TOOLBELT_VERSION="20190827.085359.7e083741fd"
WORKDIR /build/tools/
ADD "https://console.improbable.io/toolbelt/download/${TOOLBELT_VERSION}/linux" ./spatial
RUN ["chmod", "+x", "./spatial"]
ENV PATH "${PATH}:/build/tools/"

# Setup volumes to mount our secrets into.
VOLUME /var/spatial_oauth
VOLUME /var/cloudsmith_credentials
ENV IMPROBABLE_CONFIG_DIR "/var/spatial_oauth/"
ENV CLOUDSMITH_CONFIG_DIR "/var/cloudsmith_credentials"

# Copy gdk-for-unity and our entrypoint script into the container.
WORKDIR /usr/src/
COPY gdk-for-unity ./gdk-for-unity
COPY ./ci/docker/publish-packages-entrypoint.sh ./entrypoint.sh
RUN ["chmod", "+x", "./entrypoint.sh"]

ENTRYPOINT ["./entrypoint.sh"]
