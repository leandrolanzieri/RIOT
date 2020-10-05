# compile only needed files
NO_AUTO_SRC := 1
SRC := main.c

# SHA tests
ifneq (,$(SHA256)$(SHA1))
  SRC += test_sha1-sha256.c
endif

# HMAC SHA256 test
ifneq (,$(HMAC))
  SRC += test_hmac_sha256.c
endif

# AES tests
ifneq (,$(AES_CBC)$(AES_ECB)$(AES_CTR)$(TEST_AES_ECB_PARALLEL))
  SRC += test_aes.c
endif

# SHA256 energy tests
ifneq (,$(TEST_ENERGY_SHA256))
  SRC += energy_sha256.c
endif

# HMAC SHA256 energy tests
ifneq (,$(TEST_ENERGY_HMAC_SHA256))
  SRC += energy_hmac_sha256.c
endif

# SHA256 energy tests
ifneq (,$(TEST_ENERGY_AES_CBC_ENC)$(TEST_ENERGY_AES_CBC_DEC)$(TEST_ENERGY_AES_ECB_ENC)$(TEST_ENERGY_AES_ECB_DEC))
  SRC += energy_aes.c
endif
