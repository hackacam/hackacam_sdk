PACKAGE    := rtsp_server
TARGETS    := s7

SOURCES    :=               \
    a2a_communications.cpp  \
    a2a_file_source.cpp     \
    main_rtsp_server.cpp    \
    rtsp_sdk.cpp

LINK_LIBS  := rtsp sdk codecs sct svc a2a aio sbl 

ifndef ROOT
    ifdef TPT
        include $(TPT)/make/base.mk
        export ROOT := $(call find_root,.host_root)
    endif
    ifndef ROOT
        $(error variable ROOT is undefined)
    endif
endif
include $(ROOT)/make/bin.mk

CPPFLAGS += -I$(LINUX_INCL)/linux -I$(COMMON_INCL)
CXXFLAGS += -Wall -Werror -pthread
LDFLAGS  += -pthread -lrt 

build_date.h :
	touch $@

.PHONY: build_date.h
