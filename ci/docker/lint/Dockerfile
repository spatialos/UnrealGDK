FROM microsoft/dotnet:2.2-sdk

# Install the linting tool
RUN dotnet tool install --global dotnet-format
ENV PATH="${PATH}:/root/.dotnet/tools"

# Setup the volume which we will be running from
VOLUME /project
WORKDIR /project

# Entrypoint
ENTRYPOINT [ "dotnet", "format" ]
