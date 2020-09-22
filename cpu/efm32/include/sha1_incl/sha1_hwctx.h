#ifndef SHA1_HWCTX_H
#define SHA1_HWCTX_H

#include "em_cmu.h"
#include "em_crypto.h"
#include "em_device.h"
#include "kernel_defines.h"

#if (IS_ACTIVE(MODULE_PERIPH_HASH_SHA1) && IS_ACTIVE(MODULE_GECKO_SDK))

typedef struct {
    CRYPTO_SHA1_Digest_TypeDef digest;
} sha1_context;

#endif /* MODULE_PERIPH_HASH_SHA1 */
#endif /* SHA1_HWCTX_H */
