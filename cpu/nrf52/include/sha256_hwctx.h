#ifndef SHA256_HWCTX_H
#define SHA256_HWCTX_H

#include "cryptocell_incl/crys_hash.h"
#include "kernel_defines.h"

#if (IS_ACTIVE(MODULE_PERIPH_HASH_SHA256) && IS_ACTIVE(MODULE_LIB_CRYPTOCELL))

typedef struct {
    CRYS_HASHUserContext_t cc310_ctx;
} sha256_context_t;

#endif /* MODULE_PERIPH_HASH_SHA256 */
#endif /* SHA256_HWCTX_H */