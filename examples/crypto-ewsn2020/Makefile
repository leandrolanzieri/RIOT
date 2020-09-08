# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

APPLICATION = crypto-ewsn2020

FEATURES_REQUIRED += periph_gpio

ifeq (1, $(TEST_ENERGY_SHA256))
	CFLAGS +=-DTEST_ENERGY_SHA256=1
endif

ifeq (1, $(TEST_ENERGY_AES_CBC_ENC))
	CFLAGS +=-DTEST_ENERGY_AES_CBC_ENC=1
	CFLAGS +=-DTEST_ENERGY_AES_CBC=1
endif

ifeq (1, $(TEST_ENERGY_AES_CBC_DEC))
	CFLAGS +=-DTEST_ENERGY_AES_CBC_DEC=1
	CFLAGS +=-DTEST_ENERGY_AES_CBC=1
endif

ifeq (1, $(TEST_ENERGY_AES_ECB_ENC))
	CFLAGS +=-DTEST_ENERGY_AES_ECB_ENC=1
	CFLAGS +=-DTEST_ENERGY_AES_ECB=1
endif

ifeq (1, $(TEST_ENERGY_AES_ECB_DEC))
	CFLAGS +=-DTEST_ENERGY_AES_ECB_DEC=1
	CFLAGS +=-DTEST_ENERGY_AES_ECB=1
endif

ifneq (, $(TEST_ENERGY_ITER))
	CFLAGS +=-DTEST_ENERGY_ITER=TEST_ENERGY_ITER
else
	CFLAGS +=-DTEST_ENERGY_ITER=1000
endif

ifneq (, $(TEST_ENERGY%))
	USEMODULE += xtimer
endif

ifeq (pba-d-01-kw2x, $(BOARD))
	# Kinetis Hardware
	CFLAGS += -DFREESCALE_MMCAU -Wno-unused-function
	USEMODULE += lib_mmcau
endif

ifeq (frdm-k64f, $(BOARD))
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
