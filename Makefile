# Root helper targets for local PintOS debug-test workflows.

.DEFAULT_GOAL := test

DEBUG_TEST_RUNNER := pintos/tools/debug-test/run-debug-tests.py
BUILD ?= vm
TEST ?=
DEBUG ?=
TIMEOUT ?= 60
MEMORY ?= 20
SWAP_DISK ?= 4
FS_DISK ?= 2
BREAK ?=

.PHONY: test test-debug test-debug-wait test-list debug debug-wait list kself kself-debug kself-debug-wait kself-list

test kself:
	python3 $(DEBUG_TEST_RUNNER) \
		--project-root "$(CURDIR)" \
		--build "$(BUILD)" \
		--timeout "$(TIMEOUT)" \
		--memory "$(MEMORY)" \
		--swap-disk "$(SWAP_DISK)" \
		--fs-disk "$(FS_DISK)" \
		$(if $(TEST),--test "$(TEST)",) \
		$(if $(DEBUG),--debug,)

test-debug kself-debug:
	python3 $(DEBUG_TEST_RUNNER) \
		--project-root "$(CURDIR)" \
		--build "$(BUILD)" \
		--timeout "$(TIMEOUT)" \
		--memory "$(MEMORY)" \
		--swap-disk "$(SWAP_DISK)" \
		--fs-disk "$(FS_DISK)" \
		$(if $(TEST),--test "$(TEST)",) \
		$(if $(BREAK),--breakpoint "$(BREAK)",) \
		--debug \
		--debug-mode gdb

test-debug-wait kself-debug-wait:
	python3 $(DEBUG_TEST_RUNNER) \
		--project-root "$(CURDIR)" \
		--build "$(BUILD)" \
		--timeout "$(TIMEOUT)" \
		--memory "$(MEMORY)" \
		--swap-disk "$(SWAP_DISK)" \
		--fs-disk "$(FS_DISK)" \
		$(if $(TEST),--test "$(TEST)",) \
		--debug \
		--debug-mode wait

test-list kself-list:
	python3 $(DEBUG_TEST_RUNNER) \
		--project-root "$(CURDIR)" \
		--build "$(BUILD)" \
		--list

debug: test-debug

debug-wait: test-debug-wait

list: test-list
