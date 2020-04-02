FROM microsoft/dotnet:2.2-sdk as build
WORKDIR /app
COPY ./tools ./tools/
COPY ./ci/docker/entrypoint.sh ./entrypoint.sh

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get -qq install -y --no-install-recommends gosu && \
    apt-get clean

# Volume to output logs & Buildkite metadata to
VOLUME /var/logs

ENTRYPOINT ["/bin/bash", "entrypoint.sh"]
