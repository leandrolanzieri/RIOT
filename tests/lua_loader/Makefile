include ../Makefile.tests_common

USEPKG += lua

# This application uses getchar and thus expects input from stdio
USEMODULE += stdin

BOARD_WHITELIST += native samr21-xpro

ifneq ($(BOARD),native)
  # This stack size is large enough to run Lua print() functions of
  # various lengths. Other functions untested.
  CFLAGS += -DCONFIG_THREAD_STACKSIZE_MAIN='(CONFIG_THREAD_STACKSIZE_DEFAULT+2048)'
endif

include $(RIOTBASE)/Makefile.include
