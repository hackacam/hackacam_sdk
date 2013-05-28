SUBDIRS := a2asend cgi_server diag_tests rtsp_server svc_extract vrmtest psia_server dvrcp

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

vrmtest : rtsp_server
