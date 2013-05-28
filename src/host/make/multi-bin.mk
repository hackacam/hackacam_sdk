# Create multiple executable programs
# Each file in SOURCES creates a separate program
# all programs are placed under $(BIN_DIR)/$(PACKAGE)


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
CPPFLAGS    += -I$(INCL_DIR)
ifdef STATIC_LINK
    LDFLAGS += $(LINK_LIBS:%=$(LIB_DIR)/lib%.a)
else
    LDFLAGS += -L$(LIB_DIR) $(addprefix -l,$(LINK_LIBS))
endif

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

BIN_DIR     := $(BIN_DIR)/$(PACKAGE)
programs    := $(call from_sources,$(SOURCES),$(BIN_DIR),)

all_targets :=  $(programs) \
                $(addprefix $(BIN_DIR)/,$(SCRIPTS))  \
                $(addprefix $(CONF_DIR)/,$(CONFS))   \
                $(addprefix $(HTML_DIR)/,$(HTMLS))

all : $(all_targets) Makefile

$(TARGETS) : all

$(programs) : $(BIN_DIR)/% : $(OBJ_DIR)/%.o | $(BIN_DIR)/
	$(LINK) $(CPPFLAGS) $(CXXFLAGS)  $< -o $@ $(LDFLAGS)

clean: 
	rm -rf $(OBJ_DIR)

clear: clean
	rm -rf $(all_targets) $(BIN_DIR)

include $(ROOT)/make/base_rules.mk
endif

skipped_targets := $(filter-out $(TARGETS), s7 x86)
ifneq ($(skipped_targets),)

$(skipped_targets) :
	$(info ------ Skipping $@ for the package $(PACKAGE))

.PHONY : $(skipped_targets)
endif
