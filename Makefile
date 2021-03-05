TOPTARGETS := all build clean modules_install

MAKEFLAGS += --quiet

SUBDIRS := ./7seg_display

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
