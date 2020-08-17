#ifndef SHA256_HWCTX_H
#define SHA256_HWCTX_H

#include "kernel_defines.h"

#define SHA256_STATE_SIZE   (8)
#define SHA256_DIGEST_SIZE  (32)

#if (IS_ACTIVE(MODULE_PERIPH_HASH_SHA256) && IS_ACTIVE(MODULE_LIB_MMCAU))
typedef struct {
    unsigned int sha256_state[SHA256_STATE_SIZE];
    // uint8_t sha256_digest[SHA256_DIGEST_SIZE];
} sha256_context_t;
#endif /* MODULE_PERIPH_HASH_SHA256 */

#endif /* SHA256_HWCTX_H */