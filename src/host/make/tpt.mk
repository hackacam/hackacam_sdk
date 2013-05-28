# copy libaries and include file from TPT location to local client
ifndef ROOT
    $(error variable ROOT is undefined)
endif
include $(ROOT)/make/variables.mk

tpt_src_lib_dir := $(TPT)/lib/$(TARCH)
tpt_src_libs    := $(wildcard $(tpt_src_lib_dir)/*)
tpt_dst_libs    := $(subst $(tpt_src_lib_dir),$(LIB_DIR),$(tpt_src_libs))

tpt_src_inc_dir := $(TPT)/include/$(TARCH)
tpt_src_incs    := $(wildcard $(tpt_src_inc_dir)/*)
tpt_dst_incs    := $(subst $(tpt_src_inc_dir),$(INCL_DIR),$(tpt_src_incs))

all : $(tpt_dst_libs) $(tpt_dst_incs)

$(LIB_DIR)/% : $(tpt_src_lib_dir)/% | $(LIB_DIR)/
	cp -d -f $< $@

$(INCL_DIR)/% : $(tpt_src_inc_dir)/% | $(INCL_DIR)/
	cp -r -f $< $@

$(LIB_DIR)/ $(INCL_DIR)/ :
	@mkdir -p $@

clean clear:
	-rm -rf $(tpt_dst_libs) $(tpt_dst_incls) 

.PHONY : all clean clear
