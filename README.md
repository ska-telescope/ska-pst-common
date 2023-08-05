# ska-pst-common

This project provides a C++ library of common utilities and centralised management of package dependencies 
for the components of the Pulsar Timing instrument for SKA Mid and SKA Low.

## Documentation

[![Documentation Status](https://readthedocs.org/projects/ska-telescope-ska-pst-common/badge/?version=latest)](https://developer.skao.int/projects/ska-pst-common/en/latest/)

The documentation for this project, including the package description, Architecture description and the API modules can be found at SKA developer portal:  [https://developer.skao.int/projects/ska-pst-common/en/latest/](https://developer.skao.int/projects/ska-pst-common/en/latest/)

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
It is highly advised to use the containerised development environment through `make local-dev-env`. 
