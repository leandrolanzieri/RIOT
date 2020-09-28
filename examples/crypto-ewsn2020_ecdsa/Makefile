# name of the application
APPLICATION = crypto-ewsn2020_ecdh

USEPKG += relic

CFLAGS += -DTHREAD_STACKSIZE_MAIN=\(7*THREAD_STACKSIZE_DEFAULT\)

export RELIC_CONFIG_FLAGS=-DARCH=NONE -DOPSYS=NONE -DQUIET=off -DWORD=32 -DFP_PRIME=256 -DWITH="BN;MD;DV;FP;EP;CP;BC;EC" -DSEED=RIOTRND

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
