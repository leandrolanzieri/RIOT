_ALLMODULES = $(sort $(USEMODULE) $(USEPKG))

# Define MODULE_MODULE_NAME preprocessor macros for all modules.
# Note: Some modules may be using the 'IS_USED' macro, defined in
# kernel_defines.h, please refer to its documentation if you change the way
# module macros are defined.
ED = $(addprefix MODULE_,$(_ALLMODULES))
# EXTDEFINES will be put in CFLAGS_WITH_MACROS
EXTDEFINES = $(addprefix -D,$(call uppercase_and_underscore,$(ED)))

BASELIBS += $(_ALLMODULES:%=%.module)
