old-src-y := $(sort $(src-y))
src-y := $(sort $(src-y))


dirs := $(filter %/,$(src-y))
src-y := $(filter-out $(dirs),$(src-y))
makefiles := $(patsubst %/,%/Makefile.objects,$(dirs))

$(foreach m,$(makefiles),$(eval $(call source_fisher_include,$(m))))

# include myself
ifneq ($(old-src-y),$(src-y))
  include $(RIOTMAKE)/source_fisher_recursion.inc.mk
endif
