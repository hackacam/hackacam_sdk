PACKAGE    := cgi_server
TARGETS    := s7

SOURCES    := main_cgi_server.cpp   \
              build_date.cpp        \
              cgi_server.cpp        \
              cgi_param_set.cpp     \
              sdk_manager.cpp       \
              sdk_translator.cpp    \
              rtsp_server_iface.cpp \
              gateway.cpp           \
              mv_sender.cpp         \
              cgi_flash.c           \
              temperature.cpp       \
			  net_recovery.cpp      \
              watchdog.cpp          \
              roi.cpp

SCRIPTS    := start_cgi_server cgi_ifconfig
CONFS      := $(addprefix lighttpd_cgi_server.conf., s7100ipcam s7110ipcam s7120ipcam s7120ipcam_module) cgi.service
HTMLS       = $(subst $(HTML_SOURCE_DIR)/,,$(wildcard $(HTML_SOURCE_DIR)/*))

LINK_LIBS  := sbl rtsp sdk codecs sct fcgi svc aio

# Overwriting default rule to add documentation generation
cgi_all : all doc

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

CPPFLAGS += -I$(COMMON_INCL)
CXXFLAGS += -pthread -Wall -Werror
LDFLAGS  += -pthread -lrt

build_date.cpp :
	touch $@

ifneq ($(DOXYGEN),)

VERSION := $(word 1,$(STRETCH_VERSION)).$(word 2,$(STRETCH_VERSION)).$(word 4,$(STRETCH_VERSION))

doc : $(DOC_DIR)/$(PACKAGE)/index.html

$(DOC_DIR)/$(PACKAGE)/index.html : cgi_server.dox Doxyfile | $(OBJ_DIR)
	sed -e '/^OUTPUT_DIRECTORY /s%=.*$$%= $(DOC_DIR)%' \
         -e '/^HTML_OUTPUT /s%=.*$$%= $(PACKAGE)%'      \
         -e '/^PROJECT_NUMBER/s%=.*$$%= "version $(VERSION)"%' \
        Doxyfile > $(OBJ_DIR)/Doxyfile
	$(DOXYGEN) $(OBJ_DIR)/Doxyfile

else
doc :
	$(info Missing doxygen, skipping doxygen doc generation)
endif

.PHONY: build_date.cpp doc
