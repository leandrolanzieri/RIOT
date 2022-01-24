PKG_NAME=cc312_runtime
PKG_URL=https://github.com/ARM-software/cryptocell-312-runtime
PKG_VERSION=update-cc110-bu-00000-r1p4
PKG_LICENSE=BSD-3

include $(RIOTBASE)/pkg/pkg.mk

CC312_RUNTIME_MODULES =      \
    cc312_runtime_crypto_cc3x_sym \
    cc312_runtime_mbedtls \
    #

.PHONY: cc312_runtime_%

all: $(CC312_RUNTIME_MODULES)

cc312_runtime_crypto_%:
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/codesafe/src/crypto_api/$* -f $(CURDIR)/Makefile.cc312_runtime_module MODULE=$@

cc312_runtime_mbedtls:
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/codesafe/src/mbedtls_api -f $(CURDIR)/Makefile.cc312_runtime_module MODULE=$@
