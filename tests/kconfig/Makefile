include ../Makefile.tests_common

# This allows to use kconfig symbols before including Makefile.include
# Should be included after defining BOARD and RIOTBASE
include $(RIOTBASE)/makefiles/kconfig_symbols.mk

# Now we can check configurations in this makefile
ifdef CONFIG_APP_MSG_2
  $(info Message 2 is active)
else
  $(info Message 2 is NOT active)
endif

include $(RIOTBASE)/Makefile.include
