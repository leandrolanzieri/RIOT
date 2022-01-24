# name of your application
APPLICATION = hello-world

# If no BOARD is found in the environment, use this default:
BOARD ?= nrf9160dk

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

USEMODULE += od
USEMODULE += periph_cc310
# use interrupts on the peripheral
# USEMODULE += cc310_interrupt
export PKG_SOURCE_LOCAL_NRFXLIB = /home/leandro/Work/git/github/nrfconnect/sdk-nrfxlib

INCLUDES += -I$(RIOTCPU)/$(CPU)/periph/cc310_crypto/include


CFLAGS += -Og
include $(RIOTBASE)/Makefile.include
