#ifndef AES_HWCTX_H
#define AES_HWCTX_H

#include "kernel_defines.h"

#define AES128_CTX_SIZE    (20)

#if (IS_ACTIVE(MODULE_PERIPH_CRYPTO_AES) && IS_ACTIVE(MODULE_LIB_MMCAU))

typedef struct {
    uint8_t context[AES128_CTX_SIZE];
} cipher_context_t;

#endif /* MODULE_PERIPH_HASH_AES */
#endif /* AES_HWCTX_H */