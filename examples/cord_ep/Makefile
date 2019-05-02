# name of your application
APPLICATION = cord_ep

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

BOARD_INSUFFICIENT_MEMORY := arduino-duemilanove arduino-mega2560 arduino-nano \
                             arduino-uno chronos hifive1 mega-xplained msb-430 \
                             msb-430h nucleo-f030r8 nucleo-l053r8 \
                             nucleo-f031k6 nucleo-f042k6 nucleo-f303k8 \
                             nucleo-f334r8 nucleo-l031k6 stm32f0discovery \
                             telosb waspmote-pro wsn430-v1_3b wsn430-v1_4 z1

USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_icmpv6_echo

USEMODULE += cord_ep_standalone

USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += fmt

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP

# For debugging and demonstration purposes, we limit the lifetime to 60s
RD_LT ?= 60
CFLAGS += -DCORD_LT=$(RD_LT)

# If nothing else is defined, we use CoAP default port
RD_PORT ?= COAP_PORT
CFLAGS += -DRD_PORT=$(RD_PORT)

# The RD server's address must be defined by the build environment by setting
# the RD_ADDR environment variable. Per default, this value is set to the
# loopback address for enabling the build tests to successfully build this
# example.
RD_ADDR ?= \"affe::1\"
CFLAGS += -DRD_ADDR=$(RD_ADDR)

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include

# Set a custom channel if needed
ifneq (,$(filter cc110x,$(USEMODULE)))          # radio is cc110x sub-GHz
  DEFAULT_CHANNEL ?= 0
  CFLAGS += -DCC110X_DEFAULT_CHANNEL=$(DEFAULT_CHANNEL)
else
  ifneq (,$(filter at86rf212b,$(USEMODULE)))    # radio is IEEE 802.15.4 sub-GHz
    DEFAULT_CHANNEL ?= 5
    CFLAGS += -DIEEE802154_DEFAULT_SUBGHZ_CHANNEL=$(DEFAULT_CHANNEL)
  else                                          # radio is IEEE 802.15.4 2.4 GHz
    DEFAULT_CHANNEL ?= 26
    CFLAGS += -DIEEE802154_DEFAULT_CHANNEL=$(DEFAULT_CHANNEL)
  endif
endif