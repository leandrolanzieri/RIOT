NRF_DIR = $(RIOTBASE)/examples/hello-world/nrf-sdk
NRF_COMP_DIR = $(NRF_DIR)/components
# name of your application
APPLICATION = hello-world

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# Must be set to define correct context size for AES in cipher module
CFLAGS += -DCRYPTO_AES
# CFLAGS += -DAES_TIME

# Enable these to use the mmCAU library for Kinetis Boards
# CFLAGS += -DCRYPTO_AES -Wno-unused-function
# USEMODULE += lib_mmcau
# CFLAGS+=-DFREESCALE_MMCAU

# For Arm Cryptocell
CFLAGS += -DARM_CRYPTOCELL
USEMODULE += lib_cryptocell

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
