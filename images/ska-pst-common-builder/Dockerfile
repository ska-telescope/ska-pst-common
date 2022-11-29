ARG BUILD_IMAGE=""
FROM $BUILD_IMAGE as base

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

# Copy ctest2junit helper scripts
COPY ./images/ska-pst-common-builder/ctest2junit.xsl /usr/local/share/ctest2junit/
COPY ./images/ska-pst-common-builder/ctest2junit /usr/local/bin/

# Install cmake
# https://cmake.org/install/
# ARG CMAKE_SOURCE=https://github.com/Kitware/CMake/releases/download/v3.24.0-rc1/cmake-3.24.0-rc1.tar.gz
# ARG CMAKE_FILE=cmake-3.24.0-rc1.tar.gz
# ARG CMAKE_PATH=cmake-3.24.0-rc1
# WORKDIR /mnt/cmake
# RUN curl -L -O ${CMAKE_SOURCE} \
#     && tar -xvf ${CMAKE_FILE} \
#     && cd ${CMAKE_PATH} \
#     && ./bootstrap \
#     && make -j$(nproc) && make install -j$(nproc)

# Prepare environment for PROTOBUF
# ARG PROTOBUF_RELEASE_URL=https://github.com/protocolbuffers/protobuf/releases/download/v21.6
# ARG PROTOBUF_RELEASE_FILE=protobuf-cpp-3.21.6.tar.gz
# ARG PROTOBUF_RELEASE_FILE_FOLDER=protobuf-3.21.6
# ARG PROTOBUF=protobuf
# WORKDIR /mnt/${PROTOBUF}

# Download PROTOBUF source code
# RUN curl -L -O ${PROTOBUF_RELEASE_URL}/${PROTOBUF_RELEASE_FILE} \
#     && tar -xzvf ${PROTOBUF_RELEASE_FILE} \
#     && stat /mnt/${PROTOBUF}/${PROTOBUF_RELEASE_FILE_FOLDER}

# Compile PROTOBUF source code
# https://github.com/protocolbuffers/protobuf/blob/main/src/README.md#c-installation---unix
# WORKDIR /mnt/${PROTOBUF}/${PROTOBUF_RELEASE_FILE_FOLDER}
# RUN ./configure --prefix=/usr/local \
#     && make -j$(nproc) && make install -j$(nproc) \
#     && protoc --version

# ARG GOLANG=golang-go
# ARG GOLANG_VERSION=1.19.1
# ARG GOLANG_FILE=go${GOLANG_VERSION}.linux-amd64.tar.gz
# ARG GOLANG_RELEASE_FILE=https://go.dev/dl/${GOLANG_FILE}
# WORKDIR /mnt/${GOLANG}-${GOLANG_VERSION}
# RUN rm -rf /usr/local/go && \
#     curl -L -O  ${GOLANG_RELEASE_FILE} && \
#     tar -C /usr/local -xzf go${GOLANG_VERSION}.linux-amd64.tar.gz

ENV PATH $PATH:/usr/local/go/bin
ENV GOBIN /usr/local/bin

# ARG PROTOC_GEN_DOC_VERSION=1.5.1
# ARG PROTOC_GEN_DOC_RELEASE_URL=https://github.com/pseudomuto/protoc-gen-doc/archive/refs/tags/v${PROTOC_GEN_DOC_VERSION}
# ARG PROTOC_GEN_DOC_RELEASE_FILE=v${PROTOC_GEN_DOC_VERSION}.tar.gz
# ARG PROTOC_GEN_DOC_FILE_FOLDER=protoc-gen-doc-${PROTOC_GEN_DOC_VERSION}
# ARG PROTOC_GEN_DOC=protoc-gen-doc

# WORKDIR /mnt/${PROTOC_GEN_DOC}
# RUN curl -L -O ${PROTOC_GEN_DOC_RELEASE_URL}/${PROTOC_GEN_DOC_RELEASE_FILE} \
#     && tar -xvf ${PROTOC_GEN_DOC_RELEASE_FILE} \
#     && cd ${PROTOC_GEN_DOC_FILE_FOLDER} \
#     && go install ./cmd/protoc-gen-doc \
#     && /usr/local/bin/protoc-gen-doc -version

# Compile gRPC
# ARG GRPC_INSTALL=local-grpc-installation
# WORKDIR /mnt/ska-pst-common-builder
# RUN make local-grpc-installation

# Prepare environment for PSRDADA
# ARG PSRDADA=psrdada-code
# WORKDIR /mnt/${PSRDADA}-build

# Compile PSRDADA
# RUN cmake /mnt/ska-pst-common-builder/resources/${PSRDADA} -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
#     && make -j$(nproc) && make install && cp $(pwd)/Management/psrdada.pc ${LD_LIBRARY_PATH}/pkgconfig/

# Generate and compile protobuf definitions
WORKDIR /mnt/build
RUN cmake /mnt/ska-pst-common-builder/ -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
    && make -j$(nproc) && make install

# Remove cached pyyaml info file
# RUN rm /usr/lib/python3/dist-packages/PyYAML-5.3.1.egg-info

# Copy only required libraries and binaries
FROM $BUILD_IMAGE
COPY --from=base /lib /lib
COPY --from=base /lib32 /lib32
COPY --from=base /lib64 /lib64
COPY --from=base /usr /usr

ENV PATH $PATH:/usr/local/go/bin

WORKDIR /app
COPY --from=base /mnt/ska-pst-common-builder/protobuf/ /app/protobuf/

ENV DEBIAN_FRONTEND=noninteractive
ENV LD_LIBRARY_PATH=/usr/local/lib/

WORKDIR /usr/local/bin
CMD ["/bin/bash"]