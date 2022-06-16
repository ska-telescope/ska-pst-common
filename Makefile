## The following should be standard includes
# include core makefile targets for release management
include .make/base.mk

# include oci makefile targets for oci management
include .make/oci.mk

# include your own private variables for custom deployment configuration
-include PrivateRules.mak

OCI_IMAGE_BUILD_CONTEXT=$(PWD)