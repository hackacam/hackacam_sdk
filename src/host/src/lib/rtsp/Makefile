PACKAGE     := rtsp
TARGETS     := s7

SOURCES := \
    file_source.cpp     \
    rtcp.cpp            \
    rtp_streamer.cpp    \
    rtsp.cpp            \
    rtsp_parser.cpp     \
    rtsp_responder.cpp  \
    rtsp_server.cpp     \
    rtsp_talker.cpp     \
    rtsp_session_id.cpp \
    source_map.cpp      \
    rtsp_source.cpp

HEADERS    :=       \
    rtsp.h          \
    rtsp_server.h   \
    rtsp_source.h   \
    rtsp_session_id.h

CXXFLAGS = -Wall -Werror

ifndef ROOT
    ifdef TPT
        include $(TPT)/make/base.mk
        export ROOT := $(call find_root,.host_root)
    endif
    ifndef ROOT
        $(error variable ROOT is undefined)
    endif
endif
include $(ROOT)/make/lib.mk
