PKG_NAME=nrfxlib
PKG_URL=https://github.com/leandrolanzieri/sdk-nrfxlib.git
PKG_VERSION=dev/crypto/add_riot_support
PKG_LICENSE=Nordic-5-Clause

include $(RIOTBASE)/pkg/pkg.mk

ARCHIVE_NAME = libnrf_cc310_platform_0.9.12.a

all: $(BINDIR)/$(ARCHIVE_NAME)
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/crypto/nrf_cc310_platform/src -f $(CURDIR)/$(PKG_NAME).mk

# bring either the blob with or without interrupts
ifneq (,$(filter nrfxlib_interrupt,$(USEMODULE)))
  ARCHIVE_PATH = $(PKG_SOURCE_DIR)/crypto/nrf_cc310_platform/lib/cortex-m33/soft-float/$(ARCHIVE_NAME)
else
  ARCHIVE_PATH = $(PKG_SOURCE_DIR)/crypto/nrf_cc310_platform/lib/cortex-m33/soft-float/no-interrupts/$(ARCHIVE_NAME)
endif

$(BINDIR)/$(ARCHIVE_NAME): $(ARCHIVE_PATH)
	$(OBJCOPY) $< --redefine-sym=mutex_unlock=nrfxlib_mutex_unlock --redefine-sym=mutex_lock=nrfxlib_mutex_lock $@
