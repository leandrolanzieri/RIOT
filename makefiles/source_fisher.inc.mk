src-y += $(BOARDDIR)/
src-y += $(RIOTCPU)/$(CPU)/
src-y += $(RIOTBASE)/core/
src-y += $(RIOTBASE)/drivers/
src-y += $(RIOTBASE)/sys/
src-y += $(wildcard $(APPDIR)/*.c)

mkfile_path = $(lastword $(MAKEFILE_LIST))
current_dir = $(patsubst %/,%,$(dir $(mkfile_path)))

####
# Function to include a makefile during recursion. It checks if a variable named
# as the Makefile path is defined, if it is not defined it defines it and
# includes the Makefile.
#
# args: $(1): path to the Makefile to include
define source_fisher_include

ifndef $(1)
  $(1) := y
  include $(1)
endif

endef

include $(RIOTMAKE)/source_fisher_recursion.inc.mk

src-y := $(filter-out %/,$(src-y))
