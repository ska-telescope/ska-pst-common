FROM library/ubuntu:20.04 as base

ENV DEBIAN_FRONTEND=noninteractive
ENV LD_LIBRARY_PATH=/usr/local/lib/

ARG DEPENDENCIES_PATH=dependencies/ska-pst-common-builder

# Copy installation payloads
COPY . /mnt/ska-pst-common-builder
WORKDIR /mnt/ska-pst-common-builder

# Prepare environment
RUN apt-get update -y \
    && apt-get install -y make

# Install common apt dependencies
ARG PKG_CLI_PAYLOAD=${DEPENDENCIES_PATH}/apt.txt
ARG PKG_CLI_CMD=apt-get
ARG PKG_CLI_PARAMETERS='install --no-install-recommends -y'
RUN stat ${PKG_CLI_PAYLOAD} \
    && PKG_CLI_PAYLOAD=${PKG_CLI_PAYLOAD} PKG_CLI_PARAMETERS=${PKG_CLI_PARAMETERS} make local-pkg-install

# Install common pip dependencies
ARG PIP_CLI_PAYLOAD=${DEPENDENCIES_PATH}/pip.txt
RUN stat ${PIP_CLI_PAYLOAD} \
    && PIP_CLI_PAYLOAD=${PIP_CLI_PAYLOAD} make local-pip-install

# Install cmake
# https://cmake.org/install/
ARG CMAKE_SOURCE=https://github.com/Kitware/CMake/releases/download/v3.24.0-rc1/cmake-3.24.0-rc1.tar.gz
ARG CMAKE_FILE=cmake-3.24.0-rc1.tar.gz
ARG CMAKE_PATH=cmake-3.24.0-rc1
WORKDIR /mnt/cmake
RUN curl -L -O ${CMAKE_SOURCE} \
    && tar -xvf ${CMAKE_FILE} \
    && cd ${CMAKE_PATH} \
    && ./bootstrap \
    && make -j$(nproc) && make install -j$(nproc)

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
RUN ./configure --prefix=/usr/local \
    && make -j$(nproc) && make install -j$(nproc) \
    && protoc --version

# Compile gRPC
ARG GRPC_INSTALL=local-grpc-installation
WORKDIR /mnt/ska-pst-common-builder
RUN make local-grpc-installation

# Prepare environment for PSRDADA
ARG PSRDADA=psrdada-code
WORKDIR /mnt/${PSRDADA}-build

# Compile PSRDADA
RUN cmake /mnt/ska-pst-common-builder/resources/${PSRDADA} -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
    && make -j$(nproc) && make install && cp $(pwd)/Management/psrdada.pc ${LD_LIBRARY_PATH}/pkgconfig/

# Remove cached pyyaml info file
RUN rm /usr/lib/python3/dist-packages/PyYAML-5.3.1.egg-info

# Copy only required libraries and binaries
FROM ubuntu:20.04
COPY --from=base /lib /lib
COPY --from=base /lib32 /lib32
COPY --from=base /lib64 /lib64
COPY --from=base /usr /usr

WORKDIR /app
COPY --from=base /mnt/ska-pst-common-builder/protobuf/ /app/protobuf/

ENV DEBIAN_FRONTEND=noninteractive
ENV LD_LIBRARY_PATH=/usr/local/lib/

WORKDIR /usr/local/bin
CMD ["/bin/bash"]