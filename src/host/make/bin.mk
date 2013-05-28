# Create executable programs

# With one target, we go can skip submakes iterating over TARCHs
ifeq ($(words $(TARGETS)),1)
    TARCH := $(TARGETS)
endif

ifndef TARCH

all clean clear : $(TARGETS)

$(TARGETS) :
	@$(MAKE) --no-print-directory $(MAKECMDGOALS) TARCH=$@

.PHONY : all clean clear $(TARGETS) 

else
include $(ROOT)/make/variables.mk
$(call is_undefined,PACKAGE)
$(call is_undefined,TARGETS)

LINK        ?= $(CXX)
CXXFLAGS    += -MD -MP
CFLAGS      += -MD -MP
CPPFLAGS    += -I$(INCL_DIR) -DP4_CL=$(P4_CL)
ifdef STATIC_LINK
    lib_deps := $(LINK_LIBS:%=$(LIB_DIR)/lib%.a)
    LDFLAGS  += $(lib_deps)
else
    lib_deps := $(LINK_LIBS:%=$(LIB_DIR)/lib%.so)
    LDFLAGS  += -L$(LIB_DIR) $(addprefix -l,$(LINK_LIBS))
endif

ifdef DEBUG
    CXXFLAGS += -g
    CFLAGS   += -g
    LDFLAGS  += -rdynamic
    CPPFLAGS += -DDEBUG=1
else
    CXXFLAGS += -O3
    CFLAGS   += -O3
endif

# Add command line flags
CPPFLAGS    += $(X_CPPFLAGS)
CXXFLAGS    += $(X_CXXFLAGS)
CFLAGS      += $(X_CFLAGS)
LDFLAGS     += $(X_LDFLAGS)

all_targets :=  $(BIN_DIR)/$(PACKAGE) \
                $(addprefix $(INCL_DIR)/,$(HEADERS)) \
                $(addprefix $(BIN_DIR)/,$(SCRIPTS))  \
                $(addprefix $(CONF_DIR)/,$(CONFS))   \
                $(addprefix $(HTML_DIR)/,$(HTMLS))

all : $(all_targets) Makefile

$(TARGETS) : all

$(BIN_DIR)/$(PACKAGE) : $(OBJECTS) $(lib_deps) | $(BIN_DIR)/
	$(LINK) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -rf $(OBJ_DIR)

clear: clean
	rm -rf $(all_targets) $(DOC_DIR)/$(PACKAGE)

include $(ROOT)/make/base_rules.mk
endif

skipped_targets := $(filter-out $(TARGETS), s7 x86)
ifneq ($(skipped_targets),)

$(skipped_targets) :
	$(info ------ Skipping $@ for the package $(PACKAGE))

.PHONY : $(skipped_targets)
endif
