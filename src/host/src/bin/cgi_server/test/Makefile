TARCH ?= x86
PACKAGE := cgi_server

SOURCES := \
    test_cgi_flash.c \
    cgi_flash.c \
    mv_receiver.cpp \
    test_roi.cpp \
    roi.cpp

ifndef ROOT
    ifdef TPT
        include $(TPT)/make/base.mk
        export ROOT := $(call find_root,.host_root)
    endif
    ifndef ROOT
        $(error variable ROOT is undefined)
    endif
endif
include $(ROOT)/make/variables.mk
include $(ROOT)/make/base_rules.mk

CPPFLAGS += -I$(INCL_DIR) -I..
LDFLAGS  += -L$(LIB_DIR) -lsbl -lrt

VPATH = ..

tests := $(TEST_DIR)/mv_receiver $(TEST_DIR)/test_roi
ifeq ($(TARCH),s7)
    tests += $(TEST_DIR)/test_cgi_flash 
endif

.DEFAULT_GOAL := all

all : $(tests)

$(TEST_DIR)/mv_receiver : $(OBJ_DIR)/mv_receiver.o | $(TEST_DIR)/
	$(CXX) $(CPPFLAGS) $(CFLAGS)  $^ -o $@ $(LDFLAGS)

$(TEST_DIR)/test_roi : $(OBJ_DIR)/test_roi.o $(OBJ_DIR)/roi.o | $(TEST_DIR)/
	$(CXX) $(CPPFLAGS) $(CFLAGS)  $^ -o $@ $(LDFLAGS)

$(TEST_DIR)/test_cgi_flash : $(OBJ_DIR)/test_cgi_flash.o $(OBJ_DIR)/cgi_flash.o | $(TEST_DIR)/
	$(CC) $(CPPFLAGS) $(CFLAGS)  $^ -o $@ $(LDFLAGS)

clean:
	-rm -f $(OBJECTS) $(OBJ_DEPS)

clear: clean
	-rm -f $(tests)

.PHONY : clean clear all
