FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
ENV LD_LIBRARY_PATH=/usr/local/lib/

ARG DEPENDENCIES_PATH=dependencies/ska-pst-common-builder

# Copy installation payloads
COPY . /mnt/ska-pst-common-builder
WORKDIR /mnt/ska-pst-common-builder

# Prepare environment
RUN apt-get update -y \
    && apt-get install -y make=4.2.1-1.2

# Install common apt dependencies
ARG PKG_CLI_PAYLOAD=${DEPENDENCIES_PATH}/apt.txt
ARG PKG_CLI_CMD=apt-get
ARG PKG_CLI_PARAMETERS='install --no-install-recommends -y'
RUN stat ${PKG_CLI_PAYLOAD} \
    && PKG_CLI_PAYLOAD=${PKG_CLI_PAYLOAD} PKG_CLI_PARAMETERS=${PKG_CLI_PARAMETERS} make local-pkg-install

# Install common pip dependencies
ARG PIP_CLI_PAYLOAD=${DEPENDENCIES_PATH}/pip.txt
RUN stat ${PIP_CLI_PAYLOAD} \
    && PIP_CLI_PAYLOAD=${PIP_CLI_PAYLOAD} PIP_CLI_PARAMETERS=${PIP_CLI_PARAMETERS} make local-pkg-install

# Prepare environment for PROTOBUF
ARG PROTOBUF_RELEASE_URL=https://github.com/protocolbuffers/protobuf/releases/download/v21.1
ARG PROTOBUF_RELEASE_FILE=protobuf-cpp-3.21.1.tar.gz
ARG PROTOBUF_RELEASE_FILE_FOLDER=protobuf-3.21.1
ARG PROTOBUF=protobuf
WORKDIR /mnt/${PROTOBUF}

# Download PROTOBUF source code
RUN curl -L -O ${PROTOBUF_RELEASE_URL}/${PROTOBUF_RELEASE_FILE} \
    && tar -xzvf ${PROTOBUF_RELEASE_FILE} \
    && stat /mnt/${PROTOBUF}/${PROTOBUF_RELEASE_FILE_FOLDER}

# Compile PROTOBUF source code
# https://github.com/protocolbuffers/protobuf/blob/main/src/README.md#c-installation---unix
WORKDIR /mnt/${PROTOBUF}/${PROTOBUF_RELEASE_FILE_FOLDER}
RUN ./configure --prefix=/opt/protobuf \
    && make && make install -j$(nproc) \
    && ln -s ln -s /opt/protobuf/bin/protoc /usr/local/bin/protoc \
    && protoc --version

# Prepare environment for PSRDADA
ARG PSRDADA=psrdada-code
WORKDIR /mnt/${PSRDADA}-build

# Compile PSRDADA
RUN cmake /mnt/ska-pst-common-builder/resources/${PSRDADA} -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
    && make && make install -j$(nproc)

# Test and Cleanup
RUN dada_db -h \
    && rm -rf /mnt/*

WORKDIR /usr/local/bin
CMD ["/bin/bash"]
