include $(ROOT)/make/test.mk

# RUN_TESTS are targets to execute a test
run_tests   := $(SOURCES:%.cpp=%)
run_tests   := $(run_tests:%.c=%)

export LD_LIBRARY_PATH := $(LIB_DIR)

test : $(run_tests)

$(run_tests) : % : $(TEST_DIR)/% $(LIB_DEPS) | $(TEST_DIR)/
	@$(@:%=$(TEST_DIR)/%)

.PHONY: $(run_tests)

