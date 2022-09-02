.PHONY: local-docker-build local-docker-run
# OCI_IMAGE=? # Declare in PrivateRules.mak for invoking local development environment local-dev-env
# OCI_TAG=`grep -m 1 -o "[0-9].*" .release` # Declare in PrivateRules.mak for invoking local development environment local-dev-env
local-dev-env:
	@echo 'OCI_IMAGE: $(OCI_IMAGE)'
	@echo 'OCI_TAG: $(OCI_TAG)'
	@$(OCI_BUILDER) run --rm -ti -v $(PWD):/mnt/$(OCI_IMAGE) -w /mnt/$(OCI_IMAGE) $(OCI_IMAGE):$(OCI_TAG) bash

local-docker-run:
	docker run -it --rm -v $(PWD):/mnt/$(PROJECT) -t $(strip $(OCI_IMAGE)):$(VERSION) $(RUN_ARGS)

ifeq (local-docker-run,$(firstword $(MAKECMDGOALS)))
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  $(eval $(RUN_ARGS):;@:)
endif