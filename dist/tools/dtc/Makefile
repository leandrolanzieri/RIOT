PKG_NAME=dtc
PKG_URL=https://git.kernel.org/pub/scm/utils/dtc/dtc.git
PKG_VERSION=aed61a428a40f86a9ed1535ba938e48cb0551418
PKG_LICENSE=GPL-v2
PKG_BUILDDIR=$(CURDIR)/bin

.PHONY: all

all: git-download
# Start build in a clean environment, so variables set by RIOT's build process
# for cross compiling a specific target platform are reset and can
# be built cleanly for the native platform.
	env -i PATH="$(PATH)" "$(MAKE)" -C "$(PKG_BUILDDIR)"
	mv $(PKG_BUILDDIR)/dtc .

include $(RIOTBASE)/pkg/pkg.mk
