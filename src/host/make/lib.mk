#Create libraries

# With one target, we go can skip submakes iterating over TARCHs
ifeq ($(words $(TARGETS)),1)
    TARCH := $(TARGETS)
endif

# Iterate over TARCHs
ifndef TARCH

all clean clear : $(TARGETS)

$(TARGETS) :
	@$(MAKE) --no-print-directory $(MAKECMDGOALS) TARCH=$@


.PHONY : all clean clear $(TARGETS) 

else
# Main Makefile, TARCH is defined, set up rules for libraries
include $(ROOT)/make/variables.mk
$(call is_undefined,PACKAGE)
$(call is_undefined,TARGETS)

LINK        ?= $(CXX)
CXXFLAGS    += -MD -MP -fPIC
CFLAGS      += -MD -MP -fPIC
CPPFLAGS    += -I$(INCL_DIR) -DP4_CL=$(P4_CL)
LDFLAGS     += -L$(LIB_DIR) $(addprefix -l,$(LINK_LIBS))
ifdef DEBUG
    CXXFLAGS += -g
    CFLAGS   += -g
else
    CXXFLAGS += -O3
    CFLAGS   += -O3
endif

# Add command line flags
CPPFLAGS    += $(X_CPPFLAGS)
CXXFLAGS    += $(X_CXXFLAGS)
CFLAGS      += $(X_CFLAGS)
LDFLAGS     += $(X_LDFLAGS)

static_name  := lib$(PACKAGE).a
shared_name  := lib$(PACKAGE).so

all_targets :=  $(LIB_DIR)/$(static_name) \
                $(LIB_DIR)/$(shared_name)  \
                $(addprefix $(INCL_DIR)/$(PACKAGE)/,$(HEADERS))

all : $(all_targets) Makefile

$(TARGETS) : all

$(LIB_DIR)/$(static_name) : $(OBJECTS) | $(LIB_DIR)/
	@-rm -f $@
	$(AR) rs $@ $^

$(LIB_DIR)/$(shared_name) : $(OBJECTS) | $(LIB_DIR)/
	$(LINK) -o $@ $^ -shared $(LDFLAGS)

clean:
	rm -rf $(OBJ_DIR)

clear: clean
	rm -rf $(all_targets) $(INCL_DIR)/$(PACKAGE) $(DOC_DIR)/$(PACKAGE)

include $(ROOT)/make/base_rules.mk
endif

skipped_targets := $(filter-out $(TARGETS), s7 x86)
ifneq ($(skipped_targets),)

$(skipped_targets) :
	$(info ------ Skipping $@ for the package $(PACKAGE))

.PHONY : $(skipped_targets)
endif
