# This is the root Kconfig
KCONFIG ?= $(RIOTBASE)/Kconfig

# Define tools to use
MENUCONFIG = $(RIOTTOOLS)/kconfiglib/menuconfig.py
GENCONFIG = $(RIOTTOOLS)/kconfiglib/genconfig.py

# .config file will contain the generated configuraion
export KCONFIG_CONFIG = $(GENERATED_DIR)/.config

menuconfig: $(KCONFIG_GENERATED_DEPENDENCIES) $(MENUCONFIG)
	@$(MENUCONFIG) $(KCONFIG)

# Build a header file with all the Kconfig configurations
$(KCONFIG_GENERATED_AUTOCONF_HEADER_C): $(KCONFIG_GENERATED_DEPENDENCIES) $(GENCONFIG)
	@mkdir -p '$(dir $@)'
	@$(GENCONFIG) --header-path $@ $(KCONFIG) --config-out $(dir $@)/.config

# Build a Kconfig file defining all used modules
$(KCONFIG_GENERATED_DEPENDENCIES): FORCE
	@mkdir -p '$(dir $@)'
	@echo $(USEMODULE) | tr '\n' ' ' | awk 'BEGIN {RS=" "}{ gsub("-", "_", $$0); printf "config MODULE_%s\n    def_bool y\n\n", toupper($$0) }' > $@

include $(RIOTMAKE)/tools/kconfiglib.inc.mk
