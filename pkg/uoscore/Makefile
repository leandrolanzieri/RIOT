PKG_NAME=uoscore
PKG_URL=https://github.com/Fraunhofer-AISEC/uoscore-uedhoc
PKG_VERSION=b57a44b5bfd115b81628903ab544bda86f86fd3a
PKG_LICENSE=MIT

include $(RIOTBASE)/pkg/pkg.mk

all:
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/modules/oscore/src -f $(CURDIR)/Makefile.uoscore
