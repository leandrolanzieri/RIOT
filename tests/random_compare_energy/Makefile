include ../Makefile.tests_common

USEMODULE += xtimer
CFLAGS += -DTHREAD_STACKSIZE_MAIN=2048*3

DISABLE_MODULE += test_utils_interactive_sync

ifneq (,$(filter samr21-xpro,$(BOARD)))
	ifneq (,$(SLEEP_ONLY))
	else
	CFLAGS+=-DATRADIO_RNG=1
	USEMODULE += gnrc_netdev_default
	USEMODULE += at86rf233
	#CS         GPIO_PIN(PB, 31)
	#INT        GPIO_PIN(PB, 0)
	#SLEEP      GPIO_PIN(PA, 20)
	#RESET      GPIO_PIN(PB, 15)
    #miso_pin = GPIO_PIN(PC, 19)
    #mosi_pin = GPIO_PIN(PB, 30)
    #clk_pin  = GPIO_PIN(PC, 18)
	endif
endif

ifneq (,$(filter nucleo-f410rb,$(BOARD)))
	ifeq (1,$(ATRADIO_RNG))
		CFLAGS+=-DATRADIO_RNG=1
		DISABLE_MODULE+=periph_hwrng
		USEMODULE += at86rf233
		CONTINUE_ON_EXPECTED_ERRORS=1
		CFLAGS += -DAT86RF2XX_PARAM_SPI=SPI_DEV\(0\) # PA(4)/5/6/7
		CFLAGS += -DAT86RF2XX_PARAM_SPI_CLK=SPI_CLK_5MHZ
		CFLAGS += -DAT86RF2XX_PARAM_CS=ARDUINO_PIN_10 # PB6
		CFLAGS += -DAT86RF2XX_PARAM_INT=ARDUINO_PIN_7 # PA8
		CFLAGS += -DAT86RF2XX_PARAM_SLEEP=ARDUINO_PIN_6 # PB10
		CFLAGS += -DAT86RF2XX_PARAM_RESET=ARDUINO_PIN_5 # PB4
	else ifeq (1,$(PRNG))
		CFLAGS+=-DPRNG=1
		DISABLE_MODULE+=periph_hwrng
		USEMODULE += random
# 		USEMODULE += prng_fortuna
		CONTINUE_ON_EXPECTED_ERRORS=1
	else ifeq (1, $(ATA))
		CFLAGS+=-DATA=1
		DISABLE_MODULE+=periph_hwrng
		CONTINUE_ON_EXPECTED_ERRORS=1
		FEATURES_REQUIRED += periph_i2c
		USEPKG += cryptoauthlib
	else
		FEATURES_REQUIRED+=periph_hwrng
	endif
endif

ifeq (1, $(RIOT_RELIC_SHA))
	CFLAGS+=-DRIOT_RELIC_SHA=1
endif

ifeq (1, $(TINYCRYPT_CTR))
	DISABLE_MODULE+=periph_hwrng
	CONTINUE_ON_EXPECTED_ERRORS=1
	CFLAGS+=-DTINYCRYPT_CTR=1
	USEPKG += tinycrypt
endif

ifneq (,$(filter pba-d-01-kw2x,$(BOARD)))
  FEATURES_REQUIRED+=periph_hwrng
endif

ifneq (,$(filter nrf52840dk,$(BOARD)))
  FEATURES_REQUIRED+=periph_hwrng
endif

ifneq (,$(filter remote-reva,$(BOARD)))
  FEATURES_REQUIRED+=periph_hwrng
endif

# # fortuna requires more stack
# ifneq (,$(filter prng_fortuna,$(USEMODULE)))
#   CFLAGS += -DTHREAD_STACKSIZE_MAIN=2048
# endif

ifneq (,$(SLEEP_ONLY))
  CFLAGS+=-DSLEEP_ONLY=${SLEEP_ONLY}
  CONTINUE_ON_EXPECTED_ERRORS=1
  DISABLE_MODULE+=periph_hwrng
endif

include $(RIOTBASE)/Makefile.include
