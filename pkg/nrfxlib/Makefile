PKG_NAME=nrfxlib
PKG_URL=https://github.com/leandrolanzieri/sdk-nrfxlib.git
PKG_VERSION=dev/crypto/add_riot_support
PKG_LICENSE=Nordic-5-Clause

include $(RIOTBASE)/pkg/pkg.mk

all: $(BINDIR)/libnrf_cc310_platform_0.9.12.a $(BINDIR)/libnrf_cc310_mbedcrypto_0.9.12.a
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/crypto/nrf_cc310_platform/src -f $(CURDIR)/$(PKG_NAME).mk

# bring either the blob with or without interrupts
ifneq (,$(filter nrfxlib_interrupt,$(USEMODULE)))
  ARCHIVE_PATH = $(PKG_SOURCE_DIR)/crypto/nrf_cc310_platform/lib/cortex-m33/soft-float/libnrf_cc310_platform_0.9.12.a
  MBED_PATH = $(PKG_SOURCE_DIR)/crypto/nrf_cc310_mbedcrypto/lib/cortex-m33/soft-float/libnrf_cc310_mbedcrypto_0.9.12.a
else
  ARCHIVE_PATH = $(PKG_SOURCE_DIR)/crypto/nrf_cc310_platform/lib/cortex-m33/soft-float/no-interrupts/libnrf_cc310_platform_0.9.12.a
  MBED_PATH = $(PKG_SOURCE_DIR)/crypto/nrf_cc310_mbedcrypto/lib/cortex-m33/soft-float/no-interrupts/libnrf_cc310_mbedcrypto_0.9.12.a
endif

$(BINDIR)/libnrf_cc310_platform_0.9.12.a: $(ARCHIVE_PATH)
	$(OBJCOPY) $< --redefine-sym=mutex_unlock=nrfxlib_mutex_unlock --redefine-sym=mutex_lock=nrfxlib_mutex_lock $@

$(BINDIR)/libnrf_cc310_mbedcrypto_0.9.12.a: $(MBED_PATH)
	cp $< $@
