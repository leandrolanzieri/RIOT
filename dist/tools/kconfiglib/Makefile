PKG_NAME=kconfiglib
PKG_URL=https://github.com/ulfalizer/Kconfiglib
PKG_VERSION=94d121f8dc8faf0771b657a9f01556d7adbc3c70
PKG_LICENSE=ISC
PKG_BUILDDIR=$(CURDIR)/bin

.PHONY: all

all: git-download
	mv -t . $(PKG_BUILDDIR)/kconfiglib.py $(PKG_BUILDDIR)/menuconfig.py \
		$(PKG_BUILDDIR)/genconfig.py $(PKG_BUILDDIR)/examples/merge_config.py

include $(RIOTBASE)/pkg/pkg.mk
