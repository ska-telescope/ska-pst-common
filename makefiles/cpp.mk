BUILD_PATH=$(PWD)/build
SOURCE_PATH=$(PWD)
PROCESSOR_COUNT=nproc

# CLean build path
.PHONY: local-cpp-clean-buildpath
local-cpp-clean-buildpath:
	@rm -rf $(BUILD_PATH); mkdir -p $(BUILD_PATH)

# Build and compile
.PHONY: local-cpp-build-debug local-cpp-build-release local-cpp-build-export-compile-commands
local-cpp-build-debug:
	$(call fn_msg_start,local-cpp-build-debug)
	$(call fn_cmake,\
	$(BUILD_PATH),\
	$(SOURCE_PATH) -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DCMAKE_CXX_FLAGS="-coverage" -DCMAKE_EXE_LINKER_FLAGS="-coverage",\
	$(PROCESSOR_COUNT))
	$(call fn_msg_end,local-cpp-build-debug)

local-cpp-build-release:
	$(call fn_msg_start,local-cpp-build-release)
	$(call fn_cmake,\
	$(BUILD_PATH),\
	$(SOURCE_PATH) -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF,\
	$(PROCESSOR_COUNT))
	$(call fn_msg_end,local-cpp-build-release)

local-cpp-build-export-compile-commands:
	$(call fn_msg_start,local-cpp-export-compile-commands)
	$(call fn_cmake,\
	$(BUILD_PATH),\
	$(SOURCE_PATH) -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER=clang++,\
	$(PROCESSOR_COUNT))
	$(call fn_msg_end,local-cpp-export-compile-commands)

# 1 : $(BUILD_PATH)
# 2 : $(SOURCE_PATH)
# 3 : $(CMAKE_FLAGS)
# 4 : $(PROCESSOR_COUNT)
define fn_cmake
	@echo "fn_cmake: cd $(1) && cmake $(2) $(3) && make -j$(4)"
	cd $(1) && bash -c 'cmake $(2)'
	cd $(1) && make -j$(3)
endef

# 1 : make_target
define fn_msg_start
	@echo "START: $(1)"
endef

# 1 : make_target
define fn_msg_end
	@echo "END: $(1)"
	@echo
	@echo
endef

# Lint
LINT_CONFIG_PATH=$(PWD)/tests/lint
.PHONY: local-cpp-lint local-cpp-lint-clang local-cpp-lint-iwyu local-cpp-lint-cppcheck
local-cpp-lint-clang:
	$(call fn_msg_start,local-cpp-lint-clang)
	cd $(BUILD_PATH) && bash -c "jq '[ . - map(select(.file | contains(\"/resources/\"))) | .[] ]' compile_commands.json | sponge compile_commands.json"
	cd $(BUILD_PATH) && bash -c "ls -al /usr/bin/run-clang*"
	cd $(BUILD_PATH) && bash -c "python3 -u /usr/bin/run-clang-tidy -checks='cppcoreguidelines-*,performance-*,readibility-*,modernize-*,misc-*,clang-analyzer-*,google-*,-cppcoreguidelines-pro-type-reinterpret-cast' -quiet 2>&1 | tee clang-tidy.log"
	cd $(BUILD_PATH) && bash -c "cat clang-tidy.log | python3 $(LINT_CONFIG_PATH)/.clang-tidy-to-junit.py $(SOURCE_PATH)/ clang-tidy > $(BUILD_PATH)/clang-tidy-junit.xml"
	$(call fn_msg_end,local-cpp-lint-clang)

local-cpp-lint-iwyu:
	$(call fn_msg_start,local-cpp-lint-iwyu)
	iwyu_tool -j 2 -p $(BUILD_PATH) --  -Xiwyu --transitive_includes_only 2>&1 | tee $(BUILD_PATH)/iwyu.log
	cat $(BUILD_PATH)/iwyu.log | python3 $(LINT_CONFIG_PATH)/.clang-tidy-to-junit.py $(SOURCE_PATH)/ iwyu > $(BUILD_PATH)/iwyu-junit.xml
	$(call fn_msg_end,local-cpp-lint-iwyu)

local-cpp-lint-cppcheck:
	$(call fn_msg_start,local-cpp-lint-cppcheck)
	cd $(BUILD_PATH) && bash -c "cppcheck --check-config --xml-version=2 --enable=all --project=compile_commands.json --platform=unix64  -q --std=c++14 -i $(SOURCE_PATH)/resources --library=$(LINT_CONFIG_PATH)/.cppcheck-googletest.cfg --suppressions-list=$(LINT_CONFIG_PATH)/.cppcheck-suppressions.cfg 2>&1 | tee cppcheck.xml"
	cd $(BUILD_PATH) && bash -c "cppcheck_junit cppcheck.xml cppcheck-junit.xml"
	$(call fn_msg_end,local-cpp-lint-cppcheck)
local-cpp-lint: local-cpp-lint-clang local-cpp-lint-iwyu local-cpp-lint-cppcheck

# Test
.PHONY: local-cpp-test-ctest local-cpp-test-installation local-cpp-test-ctest-memcheck
local-cpp-test-ctest:
	$(call fn_msg_start,local-cpp-test-ctest)
	cd $(BUILD_PATH) && bash -c "ctest -T test --no-compress-output"
	$(call fn_msg_end,local-cpp-test-ctest)

PIV_COMMAND ?= test # Change this to a test command to verify compiled binary. i.e. /opt/bin/ska_pst_smrb_info
local-cpp-test-installation:
	$(call fn_msg_start,local-cpp-test-installation)
	cd $(BUILD_PATH) && bash -c "cmake $(SOURCE_PATH) -DCMAKE_INSTALL_PREFIX=/opt && make -j$(PROCESSOR_COUNT) && make install -j$(PROCESSOR_COUNT)"
	@echo
	@echo "POST INSTALLATION VERIFICATION: executing $(PIV_COMMAND)"
	@$(PIV_COMMAND)
	$(call fn_msg_end,local-cpp-test-installation)

local-cpp-test-ctest-memcheck:
	$(call fn_msg_start,local-cpp-test-memcheck)
	cd $(BUILD_PATH) && bash -c "ctest -T memcheck"
	$(call fn_msg_end,local-cpp-test-memcheck)

.PHONY: local-cpp-gcovr local-cpp-gcovr-html local-cpp-gcovr-xml
local-cpp-gcovr-html:
	$(call fn_msg_start,local-cpp-gcovr-html)
	gcovr  -r ./ -e 'src/apps/.*' -e 'resources/.*' -e '.*/CompilerIdCXX/.*' -e '.*/tests/.*' --exclude-unreachable-branches --exclude-throw-branches --html --html-details -o build/code-coverage.html
	$(call fn_msg_end,local-cpp-gcovr-html)

local-cpp-gcovr-xml:
	$(call fn_msg_start,local-cpp-gcovr-xml)
	gcovr -r ./ -e 'src/apps/.*' -e 'resources/.*' -e '.*/CompilerIdCXX/.*' -e '.*/tests/.*' --exclude-unreachable-branches --exclude-throw-branches --xml -o build/code-coverage.xml
	$(call fn_msg_end,local-cpp-gcovr-xml)

local-cpp-gcovr: local-cpp-gcovr-html local-cpp-gcovr-xml

.PHONY: local-cpp-ci-simulation local-cpp-ci-simulation-lint local-cpp-ci-simulation-test local-cpp-ci-simulation-installation local-cpp-ci-simulation-coverage
local-cpp-ci-simulation-lint: local-cpp-clean-buildpath local-cpp-build-export-compile-commands
	$(call fn_msg_start,local-cpp-ci-simulation-lint)
	$(MAKE) local-cpp-lint
	@echo "CHECK: ci_simulation_lint artefacts"
	stat $(BUILD_PATH)/clang-tidy.log
	stat $(BUILD_PATH)/clang-tidy-junit.xml
	stat $(BUILD_PATH)/iwyu.log
	stat $(BUILD_PATH)/iwyu-junit.xml
	stat $(BUILD_PATH)/cppcheck.xml
	stat $(BUILD_PATH)/cppcheck-junit.xml
	$(call fn_msg_end,local-cpp-ci-simulation-lint)

local-cpp-ci-simulation-test: local-cpp-clean-buildpath
	$(call fn_msg_start,local-cpp-ci-simulation-test)
	$(MAKE) local-cpp-build-debug
	$(MAKE) local-cpp-test-ctest
	$(MAKE) local-cpp-test-ctest-memcheck
	@echo "CHECK: ci_simulation_test artefacts"
	stat $(BUILD_PATH)/Testing/Temporary/MemoryChecker.*.log
	$(call fn_msg_end,local-cpp-ci-simulation-test)

local-cpp-ci-simulation-installation: local-cpp-clean-buildpath
	$(call fn_msg_start,local-cpp-ci-simulation-installation)
	$(MAKE) local-cpp-build-release
	$(MAKE) local-cpp-test-installation
	$(call fn_msg_end,local-cpp-ci-simulation-installation)

local-cpp-ci-simulation-coverage:
	$(call fn_msg_start,local-cpp-ci-simulation-coverage)
	$(MAKE) local-cpp-gcovr
	$(call fn_msg_end,local-cpp-ci-simulation-coverage)

local-cpp-ci-simulation:
	$(call fn_msg_start,local-cpp-ci-simulation)
	$(MAKE) local-cpp-ci-simulation-lint
	@echo "CLEANUP: local-cpp-ci-simulation-lint artefacts"
	$(MAKE) local-cpp-clean-buildpath
	$(MAKE) local-cpp-ci-simulation-test
	$(MAKE) local-cpp-ci-simulation-coverage
	@echo "CLEANUP: local-cpp-ci-simulation-test artefacts"
	$(MAKE) local-cpp-clean-buildpath
	$(call fn_msg_end,local-cpp-ci-simulation)
