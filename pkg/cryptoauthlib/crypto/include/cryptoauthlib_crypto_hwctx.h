#ifndef CRYPTOAUTHLIB_CRYPTO_HWCTX_H
#define CRYPTOAUTHLIB_CRYPTO_HWCTX_H

#include <stdint.h>

typedef struct {
    uint8_t ctx[16];
} cipher_context_t;

#endif /* CRYPTOAUTHLIB_CRYPTO_HWCTX_H */
