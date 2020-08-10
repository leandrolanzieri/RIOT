src-y += $(BOARDDIR)/
src-y += $(RIOTCPU)/$(CPU)/
src-y += $(RIOTBASE)/core/
src-y += $(RIOTBASE)/drivers/
src-y += $(RIOTBASE)/sys/
src-y += $(wildcard $(APPDIR)/*.c)

mkfile_path = $(lastword $(MAKEFILE_LIST))
current_dir = $(patsubst %/,%,$(dir $(mkfile_path)))

include $(RIOTMAKE)/source_fisher_recursion.inc.mk

src-y := $(filter-out %/,$(src-y))
