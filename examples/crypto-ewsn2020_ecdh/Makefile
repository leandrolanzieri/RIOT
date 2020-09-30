# name of the application
APPLICATION = crypto-ewsn2020_ecdh

USEPKG += relic
FEATURES_REQUIRED = periph_hwrng

ifeq (1, $(COSY_TEST))
	CFLAGS += -DCOSY_TEST=1
else
	CFLAGS +=-DDEVELHELP=1
	USEMODULE += ps
	USEMODULE += xtimer
	FEATURES_REQUIRED += periph_gpio
endif

CFLAGS += -DTHREAD_STACKSIZE_MAIN=\(5*THREAD_STACKSIZE_DEFAULT\)

export RELIC_CONFIG_FLAGS=-DARCH=NONE -DOPSYS=NONE -DQUIET=off -DWORD=32 -DFP_PRIME=256 -DWITH="BN;MD;DV;FP;EP;CP;BC;EC" -DSEED=RIOTRND

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

include $(RIOTBASE)/Makefile.include
