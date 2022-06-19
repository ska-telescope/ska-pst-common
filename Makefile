## The following should be standard includes
# include core makefile targets for release management
include .make/base.mk

# include oci makefile targets for oci management
include .make/oci.mk

# include your own private variables for custom deployment configuration
-include PrivateRules.mak

OCI_IMAGE_BUILD_CONTEXT=$(PWD)
# OCI_IMAGE=? # Declare in PrivateRules.mak for invoking local development environment local-dev-env
# OCI_TAG=`grep -m 1 -o "[0-9].*" .release` # Declare in PrivateRules.mak for invoking local development environment local-dev-env
local-dev-env:
	@echo 'OCI_IMAGE: $(OCI_IMAGE)'
	@echo 'OCI_TAG: $(OCI_TAG)'
	@$(OCI_BUILDER) run -ti -v $(PWD):/mnt/$(OCI_IMAGE) -w /mnt/$(OCI_IMAGE) $(OCI_IMAGE):$(OCI_TAG) bash

# OS package installation
.PHONY: local-pkg-install
PKG_CLI_CMD ?=			# Package manager executable
PKG_CLI_PAYLOAD ?= 		# Payload file
PKG_CLI_PARAMETERS ?= 	# Package manager installation parameters

local-pkg-install:
	$(PKG_CLI_CMD) $(PKG_CLI_PARAMETERS) `cat $(PKG_CLI_PAYLOAD)`

# Python package installation
.PHONY: local-pip-install
PIP_CLI_CMD ?=pip3				# Package manager executable
PIP_CLI_PAYLOAD ?= 				# Payload file
PIP_CLI_PARAMETERS ?=install -r	# Package manager installation parameters

local-pip-install:
	$(PIP_CLI_CMD) $(PIP_CLI_PARAMETERS) $(PIP_CLI_PAYLOAD)


.PHONY: local-proto-test local-proto-pre-test local-proto-do-test local-proto-post-test


local-proto-test: local-proto-pre-test local-proto-do-test local-proto-post-test