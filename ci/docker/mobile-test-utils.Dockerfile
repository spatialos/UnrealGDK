FROM ubuntu:18.04

# Copy everything and build
WORKDIR /app
COPY ./ci ./
RUN echo "deb [signed-by=/usr/share/keyrings/cloud.google.gpg] http://packages.cloud.google.com/apt cloud-sdk main" | tee -a /etc/apt/sources.list.d/google-cloud-sdk.list && curl https://packages.cloud.google.com/apt/doc/apt-key.gpg | apt-key --keyring /usr/share/keyrings/cloud.google.gpg  add - && apt-get update -y && apt-get install google-cloud-sdk -y

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

COPY ./ci/docker/mobile-test-tools-entrypoint.sh ./entrypoint.sh

RUN ["chmod", "+x", "./entrypoint.sh"]

ENTRYPOINT ["./entrypoint.sh"]