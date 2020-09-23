#ifndef SHA256_HWCTX_H
#define SHA256_HWCTX_H

#include <stdint.h>

typedef struct {
    /** global state */
    uint32_t state[8];
    /** processed bytes counter */
    uint32_t count[2];
    /** data buffer */
    unsigned char buf[64];
} sha256_context_t;

#endif /* SHA256_HWCTX_H */
