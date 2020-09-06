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
 * @brief       Application to measure and compare sha1 and sha256 runtime
 *
 * @author      Lena Boeckmann <lena.boeckmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "hashes/sha1.h"
#include "sha1_hwctx.h"

#include "hashes/sha256.h"
#include "sha256_hwctx.h"

#include "periph/gpio.h"

#if SHA1 || SHA256
static const unsigned char SHA_TESTSTRING[] = "This is a teststring fore sha256";
static size_t SHA_TESTSTR_SIZE = 32;
#endif

#ifdef SHA1
    static uint8_t EXPECTED_RESULT_SHA1[] = {
        0xFD, 0xE1, 0xCD, 0x3B, 0x80,
        0xC7, 0x57, 0x9F, 0x97, 0x4F,
        0x94, 0x18, 0x98, 0x94, 0x65,
        0xA8, 0x96, 0xA6, 0xEC, 0xE6
    };

    void sha1_test(gpio_t active_gpio)
    {
        uint8_t sha1_result[SHA1_DIGEST_LENGTH];
        sha1_context ctx;

        gpio_set(active_gpio);
        sha1_init(&ctx);
        gpio_clear(active_gpio);

        gpio_set(active_gpio);
        sha1_update(&ctx, SHA_TESTSTRING, SHA_TESTSTR_SIZE);
        gpio_clear(active_gpio);

        gpio_set(active_gpio);
        sha1_final(&ctx, sha1_result);
        gpio_clear(active_gpio);

        if (memcmp(sha1_result, EXPECTED_RESULT_SHA1, SHA1_DIGEST_LENGTH) != 0) {
            printf("SHA-1 Failure\n");

            for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
                printf("%02x ", sha1_result[i]);
            }
            printf("\n");
        }
        else {
            printf("SHA-1 Success\n");
        }
    }
#endif /* SHA1 */


#ifdef SHA256
    static uint8_t EXPECTED_RESULT_SHA256[] = {
        0x65, 0x0C, 0x3A, 0xC7, 0xF9, 0x33, 0x17, 0xD3,
        0x96, 0x31, 0xD3, 0xF5, 0xC5, 0x5B, 0x0A, 0x1E,
        0x96, 0x68, 0x04, 0xE2, 0x73, 0xC3, 0x8F, 0x93,
        0x9C, 0xB1, 0x45, 0x4D, 0xC2, 0x69, 0x7D, 0x20
    };

    void sha256_test(gpio_t active_gpio)
    {
        uint8_t sha256_result[SHA256_DIGEST_LENGTH];
        sha256_context_t ctx;

        gpio_set(active_gpio);
        sha256_init(&ctx);
        gpio_clear(active_gpio);

        gpio_set(active_gpio);
        sha256_update(&ctx, SHA_TESTSTRING, SHA_TESTSTR_SIZE);
        gpio_clear(active_gpio);

        gpio_set(active_gpio);
        sha256_final(&ctx, sha256_result);
        gpio_clear(active_gpio);

        if (memcmp(sha256_result, EXPECTED_RESULT_SHA256, SHA256_DIGEST_LENGTH) != 0) {
            printf("SHA-256 Failure\n");
        }
        else {
            printf("SHA-256 Success\n");
        }
    }
#endif /* SHA256 */
