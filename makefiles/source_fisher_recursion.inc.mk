old-src-y := $(sort $(src-y))
src-y := $(sort $(src-y))


dirs := $(filter %/,$(src-y))
src-y := $(filter-out $(dirs),$(src-y))
makefiles := $(patsubst %/,%/Makefile.objects,$(dirs))

$(info -> Including the following makefiles: $(makefiles))
-include $(makefiles)

# include myself
ifneq ($(old-src-y),$(src-y))
  include $(RIOTMAKE)/source_fisher_recursion.inc.mk
endif
