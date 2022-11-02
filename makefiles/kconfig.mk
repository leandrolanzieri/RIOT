# This is the root Kconfig
KCONFIG ?= $(RIOTBASE)/Kconfig

# Include tools targets
include $(RIOTMAKE)/tools/kconfiglib.inc.mk

# Include fixdep tool
include $(RIOTMAKE)/tools/fixdep.inc.mk

# Generated dir will contain Kconfig generated configurations
GENERATED_DIR = $(BINDIR)/generated

# The sync dir will contain a tree of header files that represent Kconfig symbols
export KCONFIG_SYNC_DIR = $(GENERATED_DIR)/deps

# This file will contain all generated configuration from kconfig
export KCONFIG_GENERATED_AUTOCONF_HEADER_C = $(GENERATED_DIR)/autoconf.h

# Header for the generated header file
define KCONFIG_AUTOHEADER_HEADER
$(if $(filter MINGW% CYGWIN% MSYS%,$(OS)),/)/* RIOT Configuration File */

endef
export KCONFIG_AUTOHEADER_HEADER

# This file will contain the calculated dependencies formated in Kconfig
export KCONFIG_GENERATED_DEPENDENCIES = $(GENERATED_DIR)/Kconfig.dep

# This file will contain external module configurations
export KCONFIG_EXTERNAL_MODULE_CONFIGS = $(GENERATED_DIR)/Kconfig.external_modules

# This file will contain external package configurations
export KCONFIG_EXTERNAL_PKG_CONFIGS = $(GENERATED_DIR)/Kconfig.external_pkgs

# Add configurations that only work when running the Kconfig test so far,
# because they activate modules.
ifeq (1,$(TEST_KCONFIG))
  # This file will contain application default configurations
  KCONFIG_APP_CONFIG = $(APPDIR)/app.config.test
  # This configuration enables modules that are only available when using Kconfig
  # module modelling
  # Bring in all board specific configurations if present
  ifneq (,$(wildcard $(BOARDDIR)/$(BOARD).config))
    KCONFIG_BOARD_CONFIG += $(BOARDDIR)/$(BOARD).config
  endif
else
  # This file will contain application default configurations
  KCONFIG_APP_CONFIG = $(APPDIR)/app.config
endif

# Default and user overwritten configurations
KCONFIG_USER_CONFIG = $(APPDIR)/user.config

# This file will contain configuration using the `RIOT_CONFIG_<CONFIG>`
# environment variables. Used to enforce a rerun of GENCONFIG when environment
# changes.
KCONFIG_GENERATED_ENV_CONFIG = $(GENERATED_DIR)/env.config

# This is the output of the generated configuration. It always mirrors the
# content of KCONFIG_GENERATED_AUTOCONF_HEADER_C, and it is used to load
# configuration symbols to the build system.
KCONFIG_OUT_CONFIG = $(GENERATED_DIR)/out.config

# This file is generated by the GENCONFIG tool. It is similar to the .d
# files generated by GCC, and the idea is the same. We want to re-trigger the
# generation of KCONFIG_OUT_CONFIG and KCONFIG_GENERATED_AUTOCONF_HEADER_C
# whenever a change occurs on one of the previously used Kconfig files.
KCONFIG_OUT_DEP = $(KCONFIG_OUT_CONFIG).d

# Add configurations to merge, in ascendent priority (i.e. a file overrides the
# previous ones).
#
# KCONFIG_CPU_CONFIG, KCONFIG_BOARD_CONFIG and KCONFIG_ADD_CONFIG hold a lists
# of .config files that are merged for the initial configuration. This allows
# to split configurations in common files and share them among boards or cpus.
# Files are merged from more generic to more specific.
# This file will contain application default configurations used for Kconfig Test
MERGE_SOURCES += $(KCONFIG_CPU_CONFIG)
MERGE_SOURCES += $(KCONFIG_BOARD_CONFIG)
MERGE_SOURCES += $(KCONFIG_ADD_CONFIG)
MERGE_SOURCES += $(wildcard $(KCONFIG_APP_CONFIG))
MERGE_SOURCES += $(wildcard $(KCONFIG_USER_CONFIG))

MERGE_SOURCES += $(KCONFIG_GENERATED_ENV_CONFIG)

# Create directory to place generated files
$(GENERATED_DIR): $(if $(MAKE_RESTARTS),,$(CLEAN))
	$(Q)mkdir -p $@

# During migration this checks if Kconfig should run. It will run if any of
# the following is true:
# - A previous configuration file is present (e.g. from a previous call to
#   menuconfig)
# - menuconfig is being called
# - SHOULD_RUN_KCONFIG or TEST_KCONFIG is set
#
# By default SHOULD_RUN_KCONFIG is set if any of the following is true:
# - A file with '.config' extension is present in the application directory
# - A 'Kconfig' file is present in the application directory
#
# NOTE: This assumes that Kconfig will not generate any default configurations
# just from the Kconfig files outside the application folder (i.e. module
# configuration via Kconfig is disabled by default). Should this change, the
# check would not longer be valid, and Kconfig would have to run on every
# build.
SHOULD_RUN_KCONFIG ?= $(or $(wildcard $(APPDIR)/*.config), \
                           $(wildcard $(APPDIR)/Kconfig))

ifneq (,$(filter menuconfig, $(MAKECMDGOALS)))
  SHOULD_RUN_KCONFIG := 1
endif

ifneq (,$(if $(CLEAN),,$(wildcard $(KCONFIG_OUT_CONFIG))))
  ifeq (,$(SHOULD_RUN_KCONFIG))
    WARNING_MSG := Warning! SHOULD_RUN_KCONFIG is not set but a previous \
                   configuration file was detected (did you run \
                  `make menuconfig`?). Kconfig will run regardless.
    $(warning $(WARNING_MSG))
  endif
  SHOULD_RUN_KCONFIG := 1
endif

# When testing Kconfig we should always run it
ifeq (1,$(TEST_KCONFIG))
  SHOULD_RUN_KCONFIG := 1
endif

# Expose DEVELHELP to kconfig
ifeq (1,$(DEVELHELP))
  RIOT_CONFIG_DEVELHELP ?= y
endif

# export variable to make it visible in other Makefiles
export SHOULD_RUN_KCONFIG

ifneq (,$(SHOULD_RUN_KCONFIG))

# Include configuration symbols if available. This allows to check for Kconfig
# symbols in makefiles. Make tries to 'remake' all included files (see
# https://www.gnu.org/software/make/manual/html_node/Remaking-Makefiles.html).
-include $(KCONFIG_OUT_CONFIG)

# Add configuration header to build dependencies
BUILDDEPS += $(KCONFIG_GENERATED_AUTOCONF_HEADER_C) $(FIXDEP)

# Include configuration header when building
CFLAGS += -imacros '$(KCONFIG_GENERATED_AUTOCONF_HEADER_C)'

USEMODULE_W_PREFIX = $(addprefix USEMODULE_,$(USEMODULE))
USEPKG_W_PREFIX = $(addprefix USEPKG_,$(USEPKG))

.PHONY: menuconfig autokernel

# Opens the menuconfig interface for configuration of modules using the Kconfig
# system. It will try to update the autoconf.h, which will update if needed
# (i.e. out.config changed).
menuconfig: $(MENUCONFIG) $(KCONFIG_OUT_CONFIG)
	$(Q)KCONFIG_CONFIG=$(KCONFIG_OUT_CONFIG) $(MENUCONFIG) $(KCONFIG)
	$(MAKE) $(KCONFIG_GENERATED_AUTOCONF_HEADER_C)

dependencies-of-%:
	@autokernel -K $(RIOTBASE) revdeps $*

satisfy-%:
	@autokernel -K $(RIOTBASE) satisfy -g $*

AUTOKERNEL_CONFIG = $(APPDIR)/app.conf

generate-config: $(AUTOKERNEL_CONFIG)
	@autokernel -K $(RIOTBASE) -C $(AUTOKERNEL_CONFIG) generate-config -o $(APPDIR)/$(BOARD)_generated.config

# Variable used to conditionally depend on KCONFIG_GENERATED_DEPDENDENCIES
# When testing Kconfig module modelling this file is not needed
ifneq (1, $(TEST_KCONFIG))
  GENERATED_DEPENDENCIES_DEP = $(KCONFIG_GENERATED_DEPENDENCIES)
endif

# These rules are not included when only calling `make clean` in
# order to keep the $(BINDIR) directory clean.
ifneq (clean, $(MAKECMDGOALS))

# Build a Kconfig file defining all used modules and packages. This is done by
# defining symbols like 'USEMODULE_<MODULE_NAME>' or USEPKG_<PACKAGE_NAME> which
# default to 'y'. Then, every module and package Kconfig menu will depend on
# that symbol being set to show its options.
# Do nothing when testing Kconfig module dependency modelling.
$(KCONFIG_GENERATED_DEPENDENCIES): FORCE | $(GENERATED_DIR)
	$(Q)printf "%s " $(USEMODULE_W_PREFIX) $(USEPKG_W_PREFIX) \
	  | awk 'BEGIN {RS=" "}{ gsub("-", "_", $$0); \
	      printf "config %s\n\tbool\n\tdefault y\n", toupper($$0)}' \
	  | $(LAZYSPONGE) $(LAZYSPONGE_FLAGS) $@

KCONFIG_ENV_CONFIG = $(patsubst RIOT_%,%,$(foreach v,$(filter RIOT_CONFIG_%,$(.VARIABLES)),$(v)=$($(v))))

# Build an intermediate file based on the `RIOT_CONFIG_<CONFIG>` environment
# variables
$(KCONFIG_GENERATED_ENV_CONFIG): FORCE | $(GENERATED_DIR)
	$(Q)printf "%s\n" $(KCONFIG_ENV_CONFIG) \
	  | $(LAZYSPONGE) $(LAZYSPONGE_FLAGS) $@

# All directories in EXTERNAL_MODULES_PATHS which have a Kconfig file
EXTERNAL_MODULE_KCONFIGS ?= $(sort $(foreach dir,$(EXTERNAL_MODULE_DIRS),\
                              $(wildcard $(dir)/*/Kconfig)))
# Build a Kconfig file that source all external modules configuration
# files. Every EXTERNAL_MODULE_DIRS with a Kconfig file is written to
# KCONFIG_EXTERNAL_MODULE_CONFIGS as 'osource dir/Kconfig'
$(KCONFIG_EXTERNAL_MODULE_CONFIGS): FORCE | $(GENERATED_DIR)
	$(Q)\
	if [ -n "$(EXTERNAL_MODULE_KCONFIGS)" ] ; then  \
		printf "%s\n" $(EXTERNAL_MODULE_KCONFIGS) \
		| awk '{ printf "osource \"%s\"\n", $$0 }' \
		| $(LAZYSPONGE) $(LAZYSPONGE_FLAGS) $@ ; \
	else \
		printf "# no external modules" \
		| $(LAZYSPONGE) $(LAZYSPONGE_FLAGS) $@ ; \
	fi

# All directories in EXTERNAL_PKG_DIRS which have a subdirectory containing a
# Kconfig file.
EXTERNAL_PKG_KCONFIGS ?= $(sort $(foreach dir,$(EXTERNAL_PKG_DIRS),\
                              $(wildcard $(dir)/*/Kconfig)))
# Build a Kconfig file that sources all external packages configuration
# files. Every directory with a Kconfig file is written to KCONFIG_PKG_CONFIGS
# as 'osource dir/Kconfig'
$(KCONFIG_EXTERNAL_PKG_CONFIGS): FORCE | $(GENERATED_DIR)
	$(Q)\
	if [ -n "$(EXTERNAL_PKG_KCONFIGS)" ] ; then  \
		printf "%s\n" $(EXTERNAL_PKG_KCONFIGS) \
		| awk '{ printf "osource \"%s\"\n", $$0 }' \
		| $(LAZYSPONGE) $(LAZYSPONGE_FLAGS) $@ ; \
	else \
		printf "# no external packages" \
		| $(LAZYSPONGE) $(LAZYSPONGE_FLAGS) $@ ; \
	fi

# When the 'clean' target is called, the files inside GENERATED_DIR should be
# regenerated. For that, we conditionally change GENERATED_DIR from an 'order
# only' requisite to a normal one.
#
# - When clean is called, Make will look at the modification time of the folder
#   and will regenerate the files accordingly.
# - When clean is not called Make will ensure that the folder exists, without
#   paying attention to the modification date.
#
# This allows use to generate the files only when needed, instead of using
# FORCE.
GENERATED_DIR_DEP := $(if $(CLEAN),,|) $(GENERATED_DIR)

# Generates a .config file by merging multiple sources specified in
# MERGE_SOURCES. This will also generate KCONFIG_OUT_DEP with the list of used
# Kconfig files.
$(KCONFIG_OUT_CONFIG): $(KCONFIG_EXTERNAL_MODULE_CONFIGS) $(KCONFIG_EXTERNAL_PKG_CONFIGS)
$(KCONFIG_OUT_CONFIG): $(GENERATED_DEPENDENCIES_DEP) $(GENCONFIG) $(MERGE_SOURCES) $(GENERATED_DIR_DEP)
	$(Q) $(GENCONFIG) \
	  --config-out=$(KCONFIG_OUT_CONFIG) \
	  --file-list $(KCONFIG_OUT_DEP) \
	  --kconfig-filename $(KCONFIG) $(if $(Q),,--debug )\
	  $(if $(filter 1,$(KCONFIG_IGNORE_CONFIG_ERRORS)), --ignore-config-errors) \
	  --config-sources $(MERGE_SOURCES) && \
	  touch $(KCONFIG_OUT_CONFIG)

endif # eq (clean, $(MAKECMDGOALS))

# Generates the configuration header file which holds the same information
# as KCONFIG_OUT_CONFIG, and is used to inject the configurations during
# compilation.
#
# This will generate the 'dummy' header files needed for incremental builds.
$(KCONFIG_GENERATED_AUTOCONF_HEADER_C): $(KCONFIG_OUT_CONFIG) $(GENERATED_DIR_DEP)
	$(Q) $(GENCONFIG) \
	  --header-path $(KCONFIG_GENERATED_AUTOCONF_HEADER_C) \
	  --sync-deps $(KCONFIG_SYNC_DIR) \
	  --kconfig-filename $(KCONFIG) $(if $(Q),,--debug ) \
	   $(if $(filter 1,$(KCONFIG_IGNORE_CONFIG_ERRORS)), --ignore-config-errors) \
	  --config-sources $(KCONFIG_OUT_CONFIG) && \
	  touch $(KCONFIG_GENERATED_AUTOCONF_HEADER_C)

# Try to load the list of Kconfig files used
-include $(KCONFIG_OUT_DEP)

# capture all ERROR_ prefixed Kconfig symbols
_KCONFIG_ERROR_VARS = $(filter CONFIG_ERROR_%,$(.VARIABLES))
_KCONFIG_ERRORS = $(foreach v,$(_KCONFIG_ERROR_VARS),$($(v)))

# this checks that no Kconfig error symbols are set. These symbols are used
# to indicate invalid conditions
check-kconfig-errors: $(KCONFIG_OUT_CONFIG) $(KCONFIG_GENERATED_AUTOCONF_HEADER_C)
ifneq (,$(_KCONFIG_ERRORS))
	@$(COLOR_ECHO) "$(COLOR_RED) !! There are ERRORS in the configuration !! $(COLOR_RESET)"
	@for err in $(_KCONFIG_ERRORS); do \
	  echo "- $$err"; \
	done
	@false
endif

endif
