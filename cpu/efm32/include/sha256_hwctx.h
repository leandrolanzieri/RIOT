#ifndef SHA256_HWCTX_H
#define SHA256_HWCTX_H

#include "em_cmu.h"
#include "em_crypto.h"
#include "em_device.h"
#include "kernel_defines.h"

#if (IS_ACTIVE(MODULE_PERIPH_HASH_SHA256) && IS_ACTIVE(MODULE_GECKO_SDK))

typedef struct {
    CRYPTO_SHA256_Digest_TypeDef digest;
} sha256_context_t;

#endif /* MODULE_PERIPH_HASH_SHA256 */
#endif /* SHA256_HWCTX_H */
