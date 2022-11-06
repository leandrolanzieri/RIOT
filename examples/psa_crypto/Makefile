# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

APPLICATION = example_psa_crypto

BOARD ?= native

USEMODULE += psa_crypto
USEMODULE += psa_hash
USEMODULE += psa_hash_sha_256

USEMODULE += psa_cipher
USEMODULE += psa_cipher_aes_128_cbc

USEMODULE += psa_mac
USEMODULE += psa_mac_hmac_sha_256

USEMODULE += psa_asymmetric
USEMODULE += psa_asymmetric_ecc_p256r1

ifdef SECURE_ELEMENT
  CFLAGS += -DSECURE_ELEMENT
  CFLAGS += -DTHREAD_STACKSIZE_MAIN=\(8*THREAD_STACKSIZE_DEFAULT\)
  KCONFIG_ADD_CONFIG += $(APPDIR)/app.config.test.se
else ifdef MULTIPLE_SE
  CFLAGS += -DMULTIPLE_SE
  CFLAGS += -DSECURE_ELEMENT
  CFLAGS += -DTHREAD_STACKSIZE_MAIN=\(12*THREAD_STACKSIZE_DEFAULT\)
  KCONFIG_ADD_CONFIG += $(APPDIR)/app.config.test.se.multi
else ifdef SOFTWARE
  USEMODULE += psa_cipher_aes_128_cbc_custom_backend
  USEMODULE += psa_cipher_aes_128_cbc_backend_riot # force software backend

  USEMODULE += psa_mac_hmac_sha_256_custom_backend
  USEMODULE += psa_mac_hmac_sha_256_backend_riot # force software backend

  USEMODULE += psa_asymmetric_ecc_p256r1_custom_backend
  USEMODULE += psa_asymmetric_ecc_p256r1_backend_microecc # force software backend

  KCONFIG_ADD_CONFIG += $(APPDIR)/app.config.test.sw
else
  CFLAGS += -DTHREAD_STACKSIZE_MAIN=\(12*THREAD_STACKSIZE_DEFAULT\)
endif

SHOULD_RUN_KCONFIG :=

include $(RIOTBASE)/Makefile.include
