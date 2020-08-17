#ifndef SHA1_HWCTX_H
#define SHA1_HWCTX_H

#include "kernel_defines.h"

#define SHA1_STATE_SIZE     (16)
#define SHA1_DIGEST_SIZE    (20)

#if (IS_ACTIVE(MODULE_PERIPH_HASH_SHA1) && IS_ACTIVE(MODULE_LIB_MMCAU))
typedef struct {
    unsigned int sha1_state[SHA1_STATE_SIZE];
    // uint8_t sha1_digest[SHA1_DIGEST_SIZE];
} sha1_context;
#endif /* MODULE_PERIPH_HASH_SHA1 */

#endif /* SHA1_HWCTX_H */
