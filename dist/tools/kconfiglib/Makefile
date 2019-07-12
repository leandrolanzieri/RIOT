PKG_NAME=kconfiglib
PKG_URL=https://github.com/ulfalizer/Kconfiglib
PKG_VERSION=bf01114a0ce09b891ed04609ea7e9cc6f3ef3df2
PKG_LICENSE=ISC
PKG_BUILDDIR=$(CURDIR)/bin

.PHONY: all

all: git-download
	cp -t . $(PKG_BUILDDIR)/kconfiglib.py $(PKG_BUILDDIR)/menuconfig.py \
		$(PKG_BUILDDIR)/genconfig.py $(PKG_BUILDDIR)/examples/merge_config.py

include $(RIOTBASE)/pkg/pkg.mk
