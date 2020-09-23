#ifndef SHA256_HWCTX_H
#define SHA256_HWCTX_H

#include <stdint.h>
#include "cryptoauthlib.h"

typedef struct {
    atca_sha256_ctx_t atca_sha256_ctx;
} sha256_context_t;

typedef struct {
    atca_hmac_sha256_ctx_t atca_hmac_ctx;
} hmac_context_t;

#endif /* SHA256_HWCTX_H */
