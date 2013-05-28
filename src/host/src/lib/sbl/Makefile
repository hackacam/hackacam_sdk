PACKAGE     := sbl
TARGETS     := s7 x86

SOURCES     :=          \
    sbl_exception.cpp   \
    sbl_logger.cpp      \
    sbl_socket.cpp      \
    sbl_net.cpp         \
    sbl_param_set.cpp   \
    sbl_options.cpp

HEADERS    :=      \
    sbl_exception.h \
    sbl_logger.h    \
    sbl_map.h       \
    sbl_net.h       \
    sbl_options.h   \
    sbl_param_set.h \
    sbl_socket.h    \
    sbl_test.h      \
    sbl_thread.h

ifdef SBL_MSG_SOCKET
    CPPFLAGS += -DSBL_MSG_SOCKET=$(SBL_MSG_SOCKET)
endif
ifdef SBL_MSG_THREAD
    CPPFLAGS += -DSBL_MSG_THREAD=$(SBL_MSG_THREAD)
endif

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

LDFLAGS += -lrt
