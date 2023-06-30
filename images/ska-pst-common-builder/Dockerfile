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

# Install Poetry
RUN curl -sSL https://install.python-poetry.org | python3 -
ENV PATH="${PATH}:/root/.local/bin/"

WORKDIR /usr/local/bin
CMD ["/bin/bash"]