include ../Makefile.tests_common

# Test fails to build for these boards fails due to
# redefinition of define AES_COUNT in library, which
# is also defined in efm32 header files
BOARD_BLACKLIST := stk3600 stk3700

USEPKG += cryptoauthlib
USEMODULE += hashes
USEMODULE += xtimer

# FEATURES_REQUIRED += periph_rtt
CFLAGS=-DTHREAD_STACKSIZE_MAIN=2048

ifneq (, $(NO_I2C_RECONF))
	CFLAGS+=-DNO_I2C_RECONF=1
else
	FEATURES_OPTIONAL += periph_i2c_reconfigure
endif

export TEST_KCONFIG ?= 1

include $(RIOTBASE)/Makefile.include
