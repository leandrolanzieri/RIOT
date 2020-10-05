#ifndef CRYPTO_RUNTIME_H
#define CRYPTO_RUNTIME_H

#include "periph/gpio.h"

void sha1_test(gpio_t);
void sha256_test(gpio_t);
void hmac_sha256_test(gpio_t);
void aes_cbc_test(gpio_t);
void aes_ctr_test(gpio_t);
void aes_ecb_test(gpio_t);

void sha256_test_energy(gpio_t, gpio_t);
void hmac_sha256_test_energy(gpio_t, gpio_t);
void aes_cbc_enc_test_energy(gpio_t, gpio_t);
void aes_cbc_dec_test_energy(gpio_t, gpio_t);
void aes_ecb_enc_test_energy(gpio_t, gpio_t);
void aes_ecb_dec_test_energy(gpio_t, gpio_t);

void aes_ecb_parallel_test(gpio_t *thread1_pin, gpio_t *thread2_pin);

#ifdef BOARD_PBA_D_01_KW2X
    gpio_t active_gpio = GPIO_PIN(2, 5);
    gpio_t gpio_aes_key = GPIO_PIN(2, 6);
#endif /* BOARD_PBA_D_01_KW2X */

#ifdef BOARD_FRDM_K64F
    gpio_t active_gpio = GPIO_PIN(3, 1);
    gpio_t gpio_aes_key = GPIO_PIN(3, 3);
#endif /* BOARD_FRDM_K64F */

#ifdef BOARD_NRF52840DK
    gpio_t active_gpio = GPIO_PIN(1, 7);
    gpio_t gpio_aes_key = GPIO_PIN(1, 8);
    gpio_t gpio_sync_pin = GPIO_PIN(1, 6);
#endif /* BOARD_NRF52840DK */

#ifdef BOARD_SLSTK3402A
    gpio_t active_gpio = GPIO_PIN(0, 6);
    gpio_t gpio_aes_key = GPIO_PIN(0, 7);
    gpio_t thread1_pin = GPIO_PIN(2, 9);
    gpio_t thread2_pin = GPIO_PIN(3, 9);
#endif /* BOARD_SLSTK3402A */

#endif /* CRYPTO_RUNTIME_H */