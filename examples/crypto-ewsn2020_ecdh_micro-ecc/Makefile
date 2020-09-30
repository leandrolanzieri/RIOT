# name of the application
APPLICATION = crypto-ewsn2020_ecdh

FEATURES_REQUIRED = periph_hwrng

USEPKG += micro-ecc

ifeq (1, $(COSY_TEST))
	CFLAGS += -DCOSY_TEST=1
else
	CFLAGS +=-DDEVELHELP=1
	USEMODULE += ps
	USEMODULE += xtimer
	FEATURES_REQUIRED += periph_gpio
endif

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

include $(RIOTBASE)/Makefile.include
