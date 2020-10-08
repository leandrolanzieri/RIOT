#ifndef CRYPTO_UTIL_H
#define CRYPTO_UTIL_H

#include "em_cmu.h"
#include "em_crypto.h"
#include "mutex.h"
#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AES_SIZE_BYTES  (16U)
#define AES_SIZE_WORDS  (AES_SIZE_BYTES/4)

typedef struct {
    uint32_t sequences;     /* number of sequences to run */
    uint32_t seq_count;     /* number of sequences done */
    mutex_t sequence_lock;  /* lock to wait for all sequences to finish */
} crypto_ctx_t;

#define CRYPTO_CTX_INIT { 0, 0, MUTEX_INIT }

typedef struct {
    CRYPTO_TypeDef* dev;
    CMU_Clock_TypeDef cmu;
    IRQn_Type irq;
    mutex_t lock;
    gpio_t pin;
    int write_dma_ch;
    int read_dma_ch;
    crypto_ctx_t ctx;
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

crypto_device_t* crypto_acquire_dev(void);

crypto_device_t* crypto_get_dev_by_crypto(CRYPTO_TypeDef* crypto);

void crypto_wait_for_sequence(CRYPTO_TypeDef *dev);

void crypto_aes_128_encrypt(crypto_device_t *crypto, const uint8_t *in,
                            const uint8_t *out, size_t length);

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
