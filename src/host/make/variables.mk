is_undefined = $(if $($(1)),,$(error variable $(1) is undefined))
from_sources = $(patsubst %.cpp,$(2)/%$(3),$(filter %.cpp,$(1))) \
               $(patsubst %.c,$(2)/%$(3),$(filter %.c,$(1))) \

#check make version
ifeq ($(filter 3.81,$(firstword $(sort $(MAKE_VERSION) 3.81))),)
    $(error need make version at least 3.81, have version $(MAKE_VERSION))
endif

$(call is_undefined,TARCH)
ifeq ($(filter $(TARCH),s7 x86)),)
    $(error $(TARCH) is not a valid target)
endif

include $(ROOT)/make/target-$(TARCH).mk

COMMON_INCL := $(ROOT)/../common_include
LINUX_INCL  := $(ROOT)/src/kernel/stretch_files/include

dbg         :=
ifdef DEBUG
    dbg     := -dbg
endif
BIN_DIR     := $(ROOT)/install/bin$(dbg)/$(TARCH)
LIB_DIR     := $(ROOT)/install/lib$(dbg)/$(TARCH)
INCL_DIR    := $(ROOT)/install/include/$(TARCH)
CONF_DIR    := $(ROOT)/install/conf/$(TARCH)
HTML_DIR    := $(ROOT)/install/html/$(TARCH)
DOC_DIR     := $(ROOT)/install/doc
OBJ_DIR     := $(ROOT)/build/obj$(dbg)/$(TARCH)/$(PACKAGE)
TEST_DIR    := $(ROOT)/build/test/$(TARCH)/$(PACKAGE)

SCRIPT_SOURCE_DIR := scripts
CONF_SOURCE_DIR   := conf
HTML_SOURCE_DIR   := html

OBJECTS     := $(call from_sources,$(SOURCES),$(OBJ_DIR),.o)
OBJ_DEPS    := $(OBJECTS:%.o=%.d)

DOXYGEN     := $(shell test -x /tools/bin/doxygen && echo /tools/bin/doxygen)
ifndef P4_CL
P4_CL       := $(shell test -x /tools/bin/p4 && /tools/bin/p4 changes -m 1 2> /dev/null | cut -d ' ' -f 2 || echo 0)
endif

# get stretch version
ifndef STRETCH_VERSION
STRETCH_VERSION := $(shell awk -f $(ROOT)/make/get_version.awk $(COMMON_INCL)/stretch_version.h) $(P4_CL)
endif

