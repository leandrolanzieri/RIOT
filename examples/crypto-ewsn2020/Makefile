# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

APPLICATION = crypto-ewsn2020

# Kinetis Hardware
CFLAGS += -DFREESCALE_MMCAU -Wno-unused-function
USEMODULE += lib_mmcau

# For Arm Cryptocell
# CFLAGS += -DARM_CRYPTOCELL
# USEMODULE += lib_cryptocell

include $(RIOTBASE)/Makefile.include
