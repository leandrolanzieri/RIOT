# This is the root Kconfig
KCONFIG ?= $(RIOTBASE)/Kconfig

# Include tools targets
include $(RIOTMAKE)/tools/kconfiglib.inc.mk

# .config file will contain the generated configuraion
export KCONFIG_CONFIG = $(GENERATED_DIR)/.config

# Opens the menuconfig interface for configuration of modules using the Kconfig
# system.
menuconfig: $(KCONFIG_GENERATED_DEPENDENCIES) $(MENUCONFIG)
	@$(MENUCONFIG) $(KCONFIG)

# Build a header file with all the Kconfig configurations. genconfig will avoid
# any unnecessary rewrites of the header file if no configurations changed.
$(KCONFIG_GENERATED_AUTOCONF_HEADER_C): $(KCONFIG_GENERATED_DEPENDENCIES) $(GENCONFIG) FORCE
	@mkdir -p '$(dir $@)'
	@$(GENCONFIG) --header-path $@ $(KCONFIG)

# Build a Kconfig file defining all used modules. This is done by defining
# Kconfig variables like 'module-<module-name> = y'. Then, every module Kconfig
# menu will depend on that variable being set to show its options.
$(KCONFIG_GENERATED_DEPENDENCIES): FORCE
	@mkdir -p '$(dir $@)'
	@echo $(USEMODULE) | tr '\n' ' ' \
	| awk 'BEGIN {RS=" "}{ gsub("-", "_", $$0); printf "config MODULE_%s\n    def_bool y\n", toupper($$0)}' \
	| '$(LAZYSPONGE)' $(LAZYSPONGE_FLAGS) '$@'
