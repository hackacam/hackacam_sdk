SOURCES    := \
            test_file_source.cpp    \
            test_rtsp_parser.cpp    \
            test_rtsp_responder.cpp \
            test_tcp_server.cpp     \
            test_rtsp_server.cpp

PACKAGE     := rtsp
ifndef ROOT
    ifdef TPT
        include $(TPT)/make/base.mk
        export ROOT := $(call find_root,.host_root)
    endif
    ifndef ROOT
        $(error variable ROOT is undefined)
    endif
endif
include $(ROOT)/make/test.mk

LDFLAGS += -lsbl -lrt
