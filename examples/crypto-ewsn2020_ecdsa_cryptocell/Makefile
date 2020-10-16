# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

APPLICATION = crypto-ewsn2020_ecdsa_cryptocell

CFLAGS += -DTHREAD_STACKSIZE_MAIN=\(12*THREAD_STACKSIZE_DEFAULT\)

ifeq (1, $(TEST_ENERGY))
	CFLAGS += -DTEST_ENERGY=1
endif

ifeq (1, $(COSY_TEST))
	CFLAGS += -DCOSY_TEST=1
else
	ifeq (1, $(TEST_STACK))
		CFLAGS+=-DTEST_STACK=1
		USEMODULE += ps
		CFLAGS +=-DDEVELHELP=1
	else
		USEMODULE += xtimer
		FEATURES_REQUIRED += periph_gpio
	endif
endif

USEMODULE += lib_cryptocell

include $(RIOTBASE)/Makefile.include