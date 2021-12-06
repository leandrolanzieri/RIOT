MODULE = nrfxlib

# we want to select only RIOT sources
NO_AUTO_SRC := 1
SRC := $(wildcard *_riot.c)

include $(RIOTBASE)/Makefile.base
