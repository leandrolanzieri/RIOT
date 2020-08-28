#ifndef SHA256_HWCTX_H
#define SHA256_HWCTX_H

#include "kernel_defines.h"

#define SHA256_STATE_SIZE   (8)
#define SHA256_DIGEST_SIZE  (32)
#define SHA256_BLOCK_SIZE   (64)

#if (IS_ACTIVE(MODULE_PERIPH_HASH_SHA256) && IS_ACTIVE(MODULE_LIB_MMCAU))
typedef struct {
    unsigned int sha256_state[SHA256_STATE_SIZE];
    uint32_t buffer[SHA256_BLOCK_SIZE / sizeof(uint32_t)];
    uint32_t byte_count;
    uint32_t buffer_offset;
} sha256_context_t;
#endif /* MODULE_PERIPH_HASH_SHA256 */

#endif /* SHA256_HWCTX_H */