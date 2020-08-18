#ifndef CRYPTO_UTIL_H
#define CRYPTO_UTIL_H

#include "em_cmu.h"
#include "em_crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    CRYPTO_TypeDef* dev;
    CMU_Clock_TypeDef cmu;
} crypto_device_t;

/**
 * @brief   Get mutually exclusive access to the hardware crypto peripheral.
 *
 * In case the peripheral is busy, this function will block until the
 * peripheral is available again.
 *
 * @return Acquired device to use in CRYPTO functions
 *
 */
CRYPTO_TypeDef* crypto_acquire(void);

/**
 * @brief   Release the hardware crypto peripheral to be used by others.
 *
 * @param[in] dev           Device to release
 *
 */
void crypto_release(CRYPTO_TypeDef*);

#ifdef __cplusplus
}
#endif

#endif /* CRYPTO_UTIL_H */
/** @} */
