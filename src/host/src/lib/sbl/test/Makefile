TARCH     := x86

SOURCES     :=  test_exception.cpp  \
                test_logger1.cpp    \
                test_logger2.cpp    \
                test_logger3.cpp    \
                test_thread.cpp     \
                test_socket.cpp     \
                test_net.cpp        \
                test_map.cpp        \
                test_param_set.cpp  \
                test_options.cpp

PACKAGE         := sbl

LIB_DEPS     = $(LIB_DIR)/lib$(PACKAGE).so

ifndef ROOT
    ifdef TPT
        include $(TPT)/make/base.mk
        ROOT := $(call find_root,.host_root)
    endif
    ifndef ROOT
        $(error variable ROOT is undefined)
    endif
endif
include $(ROOT)/make/run_test.mk

