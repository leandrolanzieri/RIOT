include ../Makefile.tests_common

USEMODULE += embunit
USEMODULE += random
USEPKG += c25519

BOARD_INSUFFICIENT_MEMORY := arduino-duemilanove arduino-leonardo arduino-nano \
                             arduino-uno

TEST_ON_CI_WHITELIST += all

include $(RIOTBASE)/Makefile.include

# c25519 takes up to 1.5K in stack, almost independent of the platform
ifneq (,$(filter cortex-%,$(CPU_ARCH)))
  CFLAGS += -DCONFIG_THREAD_STACKSIZE_MAIN=\(CONFIG_THREAD_STACKSIZE_DEFAULT+CONFIG_THREAD_EXTRA_STACKSIZE_PRINTF\)
else
ifneq (,$(filter atmega_common,$(USEMODULE)))
  CFLAGS += -DCONFIG_THREAD_STACKSIZE_MAIN=\(5*CONFIG_THREAD_STACKSIZE_DEFAULT+CONFIG_THREAD_EXTRA_STACKSIZE_PRINTF\)
else
  CFLAGS += -DCONFIG_THREAD_STACKSIZE_MAIN=\(3*CONFIG_THREAD_STACKSIZE_DEFAULT+CONFIG_THREAD_EXTRA_STACKSIZE_PRINTF\)
endif
endif
