## The following should be standard includes
# include core makefile targets for release management
include .make/base.mk

# include oci makefile targets for oci management
include .make/oci.mk

# include your own private variables for custom deployment configuration
-include PrivateRules.mak

OCI_IMAGE_BUILD_CONTEXT=$(PWD)

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
