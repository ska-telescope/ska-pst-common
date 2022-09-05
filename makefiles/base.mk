# Common make targets for pst build products

PST_MAKE_PATHS=.pst/makefiles
# Containerised local development environment
include $(PST_MAKE_PATHS)/devenv.mk

# CPP development make targets
include $(PST_MAKE_PATHS)/cpp.mk
