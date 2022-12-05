ARG BUILD_IMAGE=""
FROM $BUILD_IMAGE as base

ENV DEBIAN_FRONTEND=noninteractive
ENV LD_LIBRARY_PATH=/usr/local/lib/

ARG DEPENDENCIES_PATH=dependencies/ska-pst-common-builder

# Copy installation payloads
COPY . /mnt/ska-pst-common-builder
WORKDIR /mnt/ska-pst-common-builder

WORKDIR /mnt/build
RUN cmake /mnt/ska-pst-common-builder/ -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
    && make -j$(nproc) \
    && make test \
    && rm -fr * \
    && cmake /mnt/ska-pst-common-builder/ -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
    && make -j$(nproc) && make install

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