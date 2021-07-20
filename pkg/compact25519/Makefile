PKG_NAME    = compact25519
PKG_URL     = https://github.com/DavyLandman/compact25519/releases/download
PKG_VERSION = v1.0.2
PKG_EXT     = zip
PKG_LICENSE = MIT

PKG_SOURCE_DIR ?= $(PKGDIRBASE)/$(PKG_NAME)
PKG_ZIPFILE = $(PKGDIRBASE)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_EXT)

ifneq ($(RIOTBASE),)
include $(RIOTBASE)/Makefile.base
endif

.PHONY: all clean distclean

prepare: $(PKG_SOURCE_DIR)/

all: $(PKG_SOURCE_DIR)/
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR) -f $(RIOTBASE)/Makefile.base MODULE=$(PKG_NAME)

$(PKG_SOURCE_DIR)/: $(PKGDIRBASE)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_EXT)
	$(UNZIP_HERE) -o -j -d $(PKG_SOURCE_DIR) $<

$(PKG_ZIPFILE):
	mkdir -p $(PKGDIRBASE)
	$(Q)$(DOWNLOAD_TO_FILE) $@ $(PKG_URL)/$(PKG_VERSION)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_EXT)

clean::
	# Reset package to checkout state.
	rm -rf $(PKG_SOURCE_DIR)

distclean::
	rm -rf $(PKG_SOURCE_DIR) $(PKGDIRBASE)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_EXT)
