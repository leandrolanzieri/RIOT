# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

APPLICATION = crypto-ewsn2020

ifeq (pba-d-01-kw2x, $(BOARD))
	# Kinetis Hardware
	CFLAGS += -DFREESCALE_MMCAU -Wno-unused-function
	USEMODULE += lib_mmcau
endif

ifeq (nrf52840dk, $(BOARD))
	CFLAGS += -DARM_CRYPTOCELL
	USEMODULE += lib_cryptocell
endif

export TEST_KCONFIG ?= 1

include $(RIOTBASE)/Makefile.include
