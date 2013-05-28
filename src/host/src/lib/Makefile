SUBDIRS := a2a rtsp sbl svc codecs sdk sdk_ui arm_sct

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

rtsp sct arm_sct : sbl
sdk  : codecs svc arm_sct
sdk_ui : sdk
