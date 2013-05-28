ifndef SUBDIRS
    $(error variable SUBDIRS is undefined)
endif

TARGETS := s7 x86

all clean clear $(TARGETS): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all clean clear $(SUBDIRS) $(TARGETS)
