#ifndef CRYPTOAUTHLIB_CRYPTO_HWCTX_H
#define CRYPTOAUTHLIB_CRYPTO_HWCTX_H

#include <stdint.h>
#include "cryptoauthlib.h"
#include "kernel_defines.h"

typedef struct {
#if defined(MODULE_CRYPTOAUTHLIB_CRYPTO_AES_CBC)
    atca_aes_cbc_ctx_t cbc_context;
#endif
#if defined(MODULE_CRYPTOAUTHLIB_CRYPTO_AES_CTR)
    atca_aes_ctr_ctx_t ctr_context;
#endif
    uint8_t key[16];
} cipher_context_t;

#endif /* CRYPTOAUTHLIB_CRYPTO_HWCTX_H */
