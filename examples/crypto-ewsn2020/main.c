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

#include "periph/gpio.h"
#include "periph_conf.h"

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
#endif /* BOARD_NRF52840DK */

#ifdef BOARD_SLSTK3402A
    gpio_t active_gpio = GPIO_PIN(2, 6);
    gpio_t gpio_aes_key = GPIO_PIN(2, 7);
#endif /* BOARD_SLSTK3402A */

int main(void)
{
    gpio_init(active_gpio, GPIO_OUT);
    gpio_init(gpio_aes_key, GPIO_OUT);
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
    return 1;
}