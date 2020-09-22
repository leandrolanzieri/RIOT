#ifndef CRYPTOAUTHLIB_CRYPTO_HWCTX_H
#define CRYPTOAUTHLIB_CRYPTO_HWCTX_H

#include <stdint.h>
#include "cryptoauthlib.h"
#include "kernel_defines.h"

#if defined(MODULE_CRYPTOAUTHLIB_CRYPTO_AES)

typedef struct {
#if defined(MODULE_CRYPTOAUTHLIB_CRYPTO_AES_CBC)
    atca_aes_cbc_ctx_t cbc_context;
#endif
#if defined(MODULE_CRYPTOAUTHLIB_CRYPTO_AES_CTR)
    atca_aes_ctr_ctx_t ctr_context;
#endif
    uint8_t key[16];
} cipher_context_t;
#endif /* MODULE_PERIPH_CRYPTO_AES */

/* TODO: Must depend on new modules after PR */

// #ifdef MODULE_PERIPH_HASH_SHA256
// typedef struct {
//     atca_sha256_ctx_t atca_sha256_ctx;
// } sha256_context_t;

// typedef struct {
//     atca_hmac_sha256_ctx_t atca_hmac_ctx;
// } hmac_context_t;

// #endif /* MODULE_PERIPH_HASH_SHA256 */

#endif /* CRYPTOAUTHLIB_CRYPTO_HWCTX_H */
