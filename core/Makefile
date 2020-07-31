# exclude submodule sources from *.c wildcard source selection
SRC := $(filter-out init.c mbox.c msg.c msg_bus.c panic.c thread_flags.c,$(wildcard *.c))

# enable submodules
SUBMODULES := 1
INCLUDE_BOARD_INCLUDES:=1
include $(RIOTBASE)/Makefile.base
