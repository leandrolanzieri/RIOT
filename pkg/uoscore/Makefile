PKG_NAME=uoscore
PKG_URL=https://github.com/inetrg/uoscore-uedhoc
PKG_VERSION=tinydtls_aes
PKG_LICENSE=MIT

include $(RIOTBASE)/pkg/pkg.mk

all:
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/modules/oscore/src -f $(CURDIR)/Makefile.uoscore
