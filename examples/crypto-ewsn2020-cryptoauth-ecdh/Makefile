# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# name of the application
APPLICATION = crypto-ewsn2020_cryptoauth_ecdh

USEPKG += cryptoauthlib

ifeq (1, $(COSY_TEST))
	CFLAGS += -DCOSY_TEST=1
else
	CFLAGS +=-DDEVELHELP=1
	USEMODULE += ps
	USEMODULE += xtimer
	FEATURES_REQUIRED += periph_gpio
endif

include $(RIOTBASE)/Makefile.include
