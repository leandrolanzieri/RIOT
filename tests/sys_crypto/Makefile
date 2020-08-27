include ../Makefile.tests_common

USEMODULE += embunit

USEMODULE += crypto_mode_aes_ctr
USEMODULE += crypto_mode_aes_cbc

# Avoid automatically including all .c files
NO_AUTO_SRC := 1

# List of files to compile, we'll extend this as we implement more modes
SRC := main.c
SRC += tests-crypto-modes-ctr.c
SRC += tests-crypto-modes-cbc.c

# Use Kconfig dependency resolution by default
export TEST_KCONFIG ?= 1

include $(RIOTBASE)/Makefile.include
