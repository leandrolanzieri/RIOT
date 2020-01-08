export RIOTBASE=$(shell git rev-parse --show-toplevel)
# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS      ?=
SPHINXBUILD     ?= sphinx-build
SOURCEDIR       = source
BUILDDIR        = build
SPHINX_SOURCE   = $(RIOTBASE)/doc/sphinx/source
SPHINX_EXAMPLES = $(SPHINX_SOURCE)/examples

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	mkdir -p $(SPHINX_EXAMPLES)
	rm -f $(SPHINX_EXAMPLES)/*.rst
	find $(RIOTBASE)/examples/*/*.rst | awk -v SPHINX_EXAMPLES=$(SPHINX_EXAMPLES) -F/ \
		'{print $$0 " " SPHINX_EXAMPLES "/" $$(NF-1) ".rst"}' | xargs -n2 cp
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
