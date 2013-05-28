include $(ROOT)/make/variables.mk

CLEAN       += $(OBJECTS) $(OBJ_DEPS)

LINK        ?= $(CXX)
CXXFLAGS    += -MD -MP -g
CFLAGS      += -MD -MP -g
CPPFLAGS    += -I$(INCL_DIR) -I..
LDFLAGS     += -L$(LIB_DIR) -l$(PACKAGE)

# tests are test executable
tests       := $(SOURCES:%.cpp=$(TEST_DIR)/%)
tests       := $(tests:%.c=$(TEST_DIR)/%)

all : $(tests)

$(tests) : $(TEST_DIR)/% : $(OBJ_DIR)/%.o $(LIB_DEPS) | $(TEST_DIR)/
	$(LINK) $(CPPFLAGS) $(CXXFLAGS)  $< -o $@ $(LDFLAGS)

clean:
	-rm -rf $(OBJECTS) $(OBJ_DEPS)

clear: clean
	-rm -rf $(TEST_DIR)

include $(ROOT)/make/base_rules.mk
