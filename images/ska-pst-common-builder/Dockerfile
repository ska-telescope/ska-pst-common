FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

ARG IMG_NAME=ska-pst-common-builder
ARG APT_PAYLOAD=apt.txt

# Copy installation payloads
COPY ./dependencies/${IMG_NAME} /mnt
WORKDIR /mnt

# Install apt dependencies
RUN stat ${APT_PAYLOAD} \
    && apt-get update -y \ 
    && apt-get install --no-install-recommends -y \
    $(cat ${APT_PAYLOAD})

WORKDIR /usr/local/bin
CMD ["/bin/bash"]
