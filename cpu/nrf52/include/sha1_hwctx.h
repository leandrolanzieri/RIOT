#ifndef SHA1_HWCTX_H
#define SHA1_HWCTX_H

#include "cryptocell_incl/crys_hash.h"
#include "kernel_defines.h"

#if IS_ACTIVE(MODULE_PERIPH_HASH_SHA1) && IS_ACTIVE(MODULE_LIB_CRYPTOCELL)

typedef struct {
    CRYS_HASHUserContext_t cc310_ctx;
} sha1_context;

#endif /* MODULE_PERIPH_HASH_SHA1 */
#endif /* SHA1_HWCTX_H */
