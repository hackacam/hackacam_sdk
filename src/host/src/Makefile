SUBDIRS := kernel lib bin driver

ifndef ROOT
    ifdef TPT
        include $(TPT)/make/base.mk
        export ROOT := $(call find_root,.host_root)
    endif
    ifndef ROOT
        $(error variable ROOT is undefined)
    endif
endif
include $(ROOT)/make/subdirs.mk

driver  : kernel
lib     : kernel driver
bin     : lib
