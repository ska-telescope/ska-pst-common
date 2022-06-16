FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

ARG IMG_NAME=ska-pst-common-builder
ARG APT_PAYLOAD=apt.txt
ARG PSRDADA=psrdada-code

# Copy installation payloads
COPY ./dependencies/${IMG_NAME} /mnt
WORKDIR /mnt

# Install apt dependencies
RUN stat ${APT_PAYLOAD} \
    && apt-get update -y \ 
    && apt-get install --no-install-recommends -y \
    $(cat ${APT_PAYLOAD})

# Copy psrdada submodule
COPY ./resources/${PSRDADA} /mnt/${PSRDADA}
WORKDIR /mnt/${PSRDADA}-build

# SET COMPILE TIME ENV VARS
ENV LD_LIBRARY_PATH=/usr/local/lib/

# Compile PSRDADA
RUN cmake /mnt/${PSRDADA} -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
    && make && make install

# Test and Cleanup
RUN dada_db -h \
    && rm -rf /mnt/*

WORKDIR /usr/local/bin
CMD ["/bin/bash"]
