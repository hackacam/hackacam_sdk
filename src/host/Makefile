#check make version
ifeq ($(filter 3.81,$(firstword $(sort $(MAKE_VERSION) 3.81))),)
    $(error need make version at least 3.81, have version $(MAKE_VERSION))
endif

export ROOT := $(CURDIR)

all s7 x86 : tpt 
	env PATH=/usr/bin:$$PATH $(MAKE) -C src $(MAKECMDGOALS)

tpt :
	@$(MAKE) --no-print-directory -f $(ROOT)/make/tpt.mk TARCH=s7
	@$(MAKE) --no-print-directory -f $(ROOT)/make/tpt.mk TARCH=x86

clean clear :
	$(MAKE) -C src $(MAKECMDGOALS)
	-rm -rf build

.PHONY : all clean clear tpt s7 x86

