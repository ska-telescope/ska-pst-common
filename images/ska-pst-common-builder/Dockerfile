FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

ARG DEPENDENCIES_PATH=dependencies/ska-pst-common-builder

# Copy installation payloads
COPY . /mnt
WORKDIR /mnt

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

# Prepare environment for PSRDADA
ENV LD_LIBRARY_PATH=/usr/local/lib/
ARG PSRDADA=psrdada-code
WORKDIR /mnt/${PSRDADA}-build

# Compile PSRDADA
RUN cmake /mnt/resources/${PSRDADA} -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
    && make && make install

# Test and Cleanup
RUN dada_db -h \
    && rm -rf /mnt/*

WORKDIR /usr/local/bin
CMD ["/bin/bash"]
