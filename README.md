# ska-pst-common

This project allows centralised management of package dependencies required to be propagated against downstream SKA Pulsar Timing components.

The primary artefact produced by this repository are oci images published to the centralised artefact repostory. At this time of writing, the base oci image used is `ubuntu:20.04` due to the future compatibility requirement against [NVIDIA's supported linux distributions](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html#linux-distributions).

## Build Instructions

Firstly clone this repo and submodules to your local file system

    git clone --recursive git@gitlab.com:ska-telescope/pst/ska-pst-common.git

The build instructions are inherited from the pipeline machinery present in `.make/oci.mk`. Simulating the Gitlab CI build automation can be done in 2 ways.

    # 1. Build all images
    make oci-build-all

    # 2. Build specific image
    OCI_IMAGE=ska-pst-common-builder make oci-build

The usage of the oci targets from the pipeline machinery can be confirmed through `make long-help oci`

## Local development
