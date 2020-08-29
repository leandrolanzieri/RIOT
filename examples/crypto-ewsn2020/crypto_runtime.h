#ifndef CRYPTO_RUNTIME_H
#define CRYPTO_RUNTIME_H

#include "periph/gpio.h"

void sha1_test(gpio_t);
void sha256_test(gpio_t);
void hmac_sha256_test(gpio_t);
void aes_cbc_test(gpio_t);
void aes_ctr_test(gpio_t);

#endif /* CRYPTO_RUNTIME_H */