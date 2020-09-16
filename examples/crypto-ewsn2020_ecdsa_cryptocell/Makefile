# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

APPLICATION = crypto-ewsn2020_ecdh_cryptocell

FEATURES_REQUIRED += periph_gpio

CFLAGS += -DARM_CRYPTOCELL
USEMODULE += lib_cryptocell

include $(RIOTBASE)/Makefile.include