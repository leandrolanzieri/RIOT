/*
 * Copyright (C) 2020 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Application to measure and compare crypto runtime
 *
 * @author      Lena Boeckmann <lena.boeckmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef ARM_CRYPTOCELL
#include "armcc_setup.h"
#endif

#include "crypto_runtime.h"
#include "xtimer.h"
#include "periph/gpio.h"
#include "periph_conf.h"

int main(void)
{
    gpio_init(active_gpio, GPIO_OUT);
    gpio_clear(active_gpio);
    gpio_init(gpio_aes_key, GPIO_OUT);
    gpio_clear(gpio_aes_key);

#ifdef TEST_AES_ECB_PARALLEL
    aes_ecb_parallel_test(&thread1_pin, &thread2_pin);
#endif

#if SHA256
    sha256_test(active_gpio);
#elif HMAC
    hmac_sha256_test(active_gpio);
#elif AES_CBC
    aes_cbc_test(active_gpio);
#elif AES_ECB
    aes_ecb_test(active_gpio);
#elif AES_CTR
    aes_ctr_test(active_gpio);
#endif

#if TEST_ENERGY_SHA256
    sha256_test_energy(active_gpio, gpio_aes_key);
#elif TEST_ENERGY_HMAC_SHA256
    hmac_sha256_test_energy(active_gpio, gpio_aes_key);
#elif TEST_ENERGY_AES_CBC_ENC
    aes_cbc_enc_test_energy(active_gpio, gpio_aes_key);
#elif TEST_ENERGY_AES_CBC_DEC
    aes_cbc_dec_test_energy(active_gpio, gpio_aes_key);
#elif TEST_ENERGY_AES_ECB_ENC
    aes_ecb_enc_test_energy(active_gpio, gpio_aes_key);
#elif TEST_ENERGY_AES_ECB_DEC
    aes_ecb_dec_test_energy(active_gpio, gpio_aes_key);
#endif
    return 1;
}