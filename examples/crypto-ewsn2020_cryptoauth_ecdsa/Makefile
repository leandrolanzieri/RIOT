# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# name of the application
APPLICATION = crypto-ewsn2020_cryptoauth_ecdsa

USEPKG += cryptoauthlib

ifeq (1, $(TEST_ENERGY))
	CFLAGS += -DTEST_ENERGY=1
endif

ifeq (1, $(TEST_MEM))
	CFLAGS += -DTEST_MEM=1
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

export TEST_KCONFIG ?= 1

include $(RIOTBASE)/Makefile.include
