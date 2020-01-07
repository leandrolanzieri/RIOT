# Generated dir will contain Kconfig generated configurations
# We can't use $(BINDIR) here yet
GENERATED_DIR = $(abspath $(CURDIR)/bin/$(BOARD)/generated)

# This file will contain merged configurations from MERGE_SOURCES and is the
# one that is used to generate the 'riotconf.h' header
KCONFIG_MERGED_CONFIG = $(GENERATED_DIR)/merged.config

# Include configuration symbols if available, only when not cleaning. This
# allows to check for Kconfig symbols in makefiles.
# Make tries to 'remake' all included files
# (see https://www.gnu.org/software/make/manual/html_node/Remaking-Makefiles.html).
# So if this file was included even when 'clean' is called, make would enter a
# loop, as it always is out-of-date.
# This has the side effect of requiring a Kconfig user to run 'clean' on a
# separate call (e.g. 'make clean && make all'), to get the symbols correctly.
ifeq (,$(filter clean, $(MAKECMDGOALS)))
  -include $(KCONFIG_MERGED_CONFIG)
endif
