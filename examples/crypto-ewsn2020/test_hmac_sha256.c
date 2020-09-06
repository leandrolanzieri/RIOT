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
 * @brief       Application to measure hmac-sha256 runtime
 *
 * @author      Lena Boeckmann <lena.boeckmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "hashes/sha256.h"
#include "sha256_hwctx.h"

#include "periph/gpio.h"

#ifdef HMAC
    static char HMAC_INPUT[] = {
        0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20,
        0x61, 0x20, 0x74, 0x65, 0x73, 0x74, 0x73, 0x74,
        0x72, 0x69, 0x6e, 0x67, 0x20, 0x66, 0x6f, 0x72,
        0x20, 0x68, 0x6d, 0x61, 0x63, 0x32, 0x35, 0x36
    };
    size_t HMAC_INPUT_SIZE = 32;

    static uint8_t HMAC_KEY[] = { 'k', 'e', 'y' };
    size_t KEY_SIZE = 3;

    static uint8_t EXPECTED_RESULT[] = {
        0x17, 0xd5, 0xdf, 0xc8, 0x8d, 0x9d, 0xaa, 0x89,
        0xf3, 0x34, 0x05, 0x04, 0x32, 0xf7, 0x93, 0xe2,
        0x8d, 0x35, 0xe0, 0x4f, 0xf4, 0xe8, 0x18, 0x57,
        0x52, 0xc3, 0x0b, 0xd3, 0xc2, 0x3c, 0x19, 0x0c
    };

    void hmac_sha256_test(gpio_t active_gpio)
    {
        uint8_t hmac_result[SHA256_DIGEST_LENGTH];
        hmac_context_t ctx;

        gpio_set(active_gpio);
        hmac_sha256_init(&ctx, HMAC_KEY, KEY_SIZE);
        gpio_clear(active_gpio);

        gpio_set(active_gpio);
        hmac_sha256_update(&ctx, HMAC_INPUT, HMAC_INPUT_SIZE );
        gpio_clear(active_gpio);

        gpio_set(active_gpio);
        hmac_sha256_final(&ctx, hmac_result);
        gpio_clear(active_gpio);

        if (memcmp(hmac_result, EXPECTED_RESULT, SHA256_DIGEST_LENGTH)) {
            printf("HMAC SHA-256 Failure\n");
        }
        else {
            printf("HMAC SHA-256 Success\n");
        }
    }
#endif /* HMAC */
