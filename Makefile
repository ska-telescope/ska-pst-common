## The following should be standard includes
# include core makefile targets for release management
include .make/base.mk

# include oci makefile targets for oci management
include .make/oci.mk

# include your own private variables for custom deployment configuration
-include PrivateRules.mak

NPROC=`nproc`
OCI_IMAGE_BUILD_CONTEXT=$(PWD)
# OCI_IMAGE=? # Declare in PrivateRules.mak for invoking local development environment local-dev-env
# OCI_TAG=`grep -m 1 -o "[0-9].*" .release` # Declare in PrivateRules.mak for invoking local development environment local-dev-env
local-dev-env:
	@echo 'OCI_IMAGE: $(OCI_IMAGE)'
	@echo 'OCI_TAG: $(OCI_TAG)'
	@$(OCI_BUILDER) run --rm -ti -v $(PWD):/mnt/$(OCI_IMAGE) -w /mnt/$(OCI_IMAGE) $(OCI_IMAGE):$(OCI_TAG) bash

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
PIP_CLI_PARAMETERS ?=install --ignore-installed -r	# Package manager installation parameters

local-pip-install:
	$(PIP_CLI_CMD) install --ignore-installed --upgrade pip
	$(PIP_CLI_CMD) $(PIP_CLI_PARAMETERS) $(PIP_CLI_PAYLOAD)

# Protobuf compilation and execution test
PROTOBUF_BASE_PATH=$(PWD)/tests/protobuf
PROTOBUF_BUILD_PATH=$(PROTOBUF_BASE_PATH)/build
PROTOBUF_BUILD_COMMAND=protoc --proto_path=$(PROTOBUF_BASE_PATH) --cpp_out=$(PROTOBUF_BUILD_PATH) $$(find $(PROTOBUF_PROTO_PATH) -iname "*.proto")
.PHONY: local-proto-test local-proto-pre-test local-proto-do-test local-proto-post-test
local-proto-pre-test:
	@rm -rf $(PROTOBUF_BUILD_PATH) && mkdir -p $(PROTOBUF_BUILD_PATH)
local-proto-do-test:
	@echo "Execute cmake"
	@echo "Command: cd $(PROTOBUF_BUILD_PATH) && cmake $(PROTOBUF_BASE_PATH)"
	@bash -c "cd $(PROTOBUF_BUILD_PATH) && cmake $(PROTOBUF_BASE_PATH)"
	@echo ""
	@echo ""
	@echo "Compile hello world"
	@echo "Command: cd $(PROTOBUF_BUILD_PATH) && make"
	@bash -c "cd $(PROTOBUF_BUILD_PATH) && make"
	@echo ""
	@echo "Execute hello world"
	@echo "Command: cd $(PROTOBUF_BUILD_PATH) && ./test_proto"
	@bash -c "cd $(PROTOBUF_BUILD_PATH) && ./test_proto"
local-proto-post-test:
local-proto-test: local-proto-pre-test local-proto-do-test local-proto-post-test

# gRPC build and installation
GRPC_BASE_PATH=$(PWD)/resources/grpc
GRPC_BUILD_PATH=$(GRPC_BASE_PATH)/cmake/build
.PHONY: local-grpc-installation local-grpc-pre-installation local-grpc-do-installation 
local-grpc-pre-installation:
	@rm -rf $(GRPC_BUILD_PATH) && mkdir -p $(GRPC_BUILD_PATH)
local-grpc-do-installation:
	@echo "gRPC installation"
	@cd $(GRPC_BUILD_PATH) && cmake -DgRPC_INSTALL=ON -DgRPC_PROTOBUF_PROVIDER=package  \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      $(GRPC_BASE_PATH)
	cd $(GRPC_BUILD_PATH) && make -j$(NPROC) && make install -j$(NPROC)
local-grpc-post-installation:
local-grpc-installation: local-grpc-pre-installation local-grpc-do-installation local-grpc-post-installation
