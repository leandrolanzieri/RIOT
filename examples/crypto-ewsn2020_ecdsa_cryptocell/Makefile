# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

APPLICATION = crypto-ewsn2020_ecdh_cryptocell

ifeq (1, $(COSY_TEST))
	CFLAGS += -DCOSY_TEST=1
else
	CFLAGS +=-DDEVELHELP=1
	USEMODULE += ps
	USEMODULE += xtimer
	FEATURES_REQUIRED += periph_gpio
endif

USEMODULE += lib_cryptocell

include $(RIOTBASE)/Makefile.include