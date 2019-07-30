# This is the root Kconfig
KCONFIG ?= $(RIOTBASE)/Kconfig

# Include tools targets
include $(RIOTMAKE)/tools/kconfiglib.inc.mk

# .config file will contain the generated configuraion
KCONFIG_CONFIG = $(GENERATED_DIR)/.config

# Generated dir will contain Kconfig generated configurations
GENERATED_DIR = $(BINDIR)/generated
# This file will contain all generated configuration from kconfig
KCONFIG_GENERATED_AUTOCONF_HEADER_C = $(GENERATED_DIR)/autoconf.h
# This file will contain the calculated dependencies formated in Kconfig
export KCONFIG_GENERATED_DEPENDENCIES = $(GENERATED_DIR)/Kconfig.dep

# This file will contain application default configurations
KCONFIG_APP_CONFIG = $(APPDIR)/app.config

# Default and user overwritten configurations
KCONFIG_DEFCONFIG = $(APPDIR)/defconfig

# This file will contain merged configurations from MERGE_SOURCES and is the
# one that is used to generate the 'autoconf.h' header
KCONFIG_MERGED_CONFIG = $(GENERATED_DIR)/merged.config

# Add configurations to merge, in ascendent priority (i.e. a file overrides the
# previous ones)
MERGE_SOURCES += $(wildcard $(KCONFIG_APP_CONFIG))
MERGE_SOURCES += $(wildcard $(KCONFIG_DEFCONFIG))
MERGE_SOURCES += $(wildcard $(KCONFIG_CONFIG))

# Build a Kconfig file defining all used modules. This is done by defining
# Kconfig variables like 'module-<module-name> = y'. Then, every module Kconfig
# menu will depend on that variable being set to show its options.
$(KCONFIG_GENERATED_DEPENDENCIES): FORCE
	@mkdir -p '$(dir $@)'
	$(Q)printf "%s " $(USEMODULE) \
	  | awk 'BEGIN {RS=" "}{ gsub("_", "-", $$0); printf "module-%s=y\n", $$0}' \
	  | '$(LAZYSPONGE)' $(LAZYSPONGE_FLAGS) '$@'

.PHONY: menuconfig

# Opens the menuconfig interface for configuration of modules using the Kconfig
# system.
menuconfig: $(KCONFIG_CONFIG) $(MENUCONFIG)
	$(Q)OS=$(OS) KCONFIG_CONFIG=$(KCONFIG_CONFIG) $(MENUCONFIG) $(KCONFIG)

# Generates a merged configuration file from the given sources
$(KCONFIG_CONFIG): $(MERGECONFIG) $(KCONFIG_GENERATED_DEPENDENCIES) FORCE
	$(Q)\
	if ! test -z "$(strip $(MERGE_SOURCES))"; then \
	  $(MERGECONFIG) $(KCONFIG) $@ $(MERGE_SOURCES); \
	else \
	  rm -f $@; \
	fi

# Generates a merged configuration file from the given sources
$(KCONFIG_MERGED_CONFIG): $(MERGECONFIG) $(KCONFIG_GENERATED_DEPENDENCIES) FORCE
	$(Q)\
	if ! test -z "$(strip $(MERGE_SOURCES))"; then \
	  $(MERGECONFIG) $(KCONFIG) $@ $(MERGE_SOURCES); \
	else \
	  rm -f $@; \
	fi

# Build a header file with all the Kconfig configurations. genconfig will avoid
# any unnecessary rewrites of the header file if no configurations changed.
$(KCONFIG_GENERATED_AUTOCONF_HEADER_C): $(KCONFIG_GENERATED_DEPENDENCIES) $(GENCONFIG) $(KCONFIG_MERGED_CONFIG) FORCE
	@mkdir -p '$(dir $@)'
	$(Q)KCONFIG_CONFIG=$(KCONFIG_MERGED_CONFIG) $(GENCONFIG) --header-path $@ $(KCONFIG)
