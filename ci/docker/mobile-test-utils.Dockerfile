FROM microsoft/dotnet:2.2-sdk as build

# Copy everything and build
WORKDIR /app
COPY ./ci ./
RUN dotnet publish -c Release -o out

# Build runtime image
FROM mcr.microsoft.com/dotnet/core/runtime:2.2
WORKDIR /app
COPY --from=build /app/*/out ./

# Setup GIT
RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y git && \
    curl -LSs -o /usr/local/bin/gosu -SL "https://github.com/tianon/gosu/releases/download/1.4/gosu-$(dpkg --print-architecture)" && \	
    chmod +x /usr/local/bin/gosu

# Create a volume to mount our SSH key into and configure git to use it.
VOLUME /var/ssh
# Volume to mount our Github token into.
VOLUME /var/github
# Volume to output logs & Buildkite metadata to
VOLUME /var/logs

COPY ./ci/docker/entrypoint.sh ./

RUN ["chmod", "+x", "./entrypoint.sh"]

ENTRYPOINT ["./entrypoint.sh"]