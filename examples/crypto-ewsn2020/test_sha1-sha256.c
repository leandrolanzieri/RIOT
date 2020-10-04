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

#include "hashes/sha256.h"

#include "periph/gpio.h"
#include "xtimer.h"

#if SHA1 || SHA256

// 512 bytes teststring
#ifdef INPUT_512
    static unsigned char SHA_TESTSTRING[] = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Ste";
    static size_t SHA_TESTSTR_SIZE = 512;
#else
    static const unsigned char SHA_TESTSTRING[] = "This is a teststring fore sha256";
    static size_t SHA_TESTSTR_SIZE = 32;
#endif /* INPUT_512 */
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
#ifndef INPUT_512
    static uint8_t EXPECTED_RESULT_SHA256[] = {
        0x65, 0x0C, 0x3A, 0xC7, 0xF9, 0x33, 0x17, 0xD3,
        0x96, 0x31, 0xD3, 0xF5, 0xC5, 0x5B, 0x0A, 0x1E,
        0x96, 0x68, 0x04, 0xE2, 0x73, 0xC3, 0x8F, 0x93,
        0x9C, 0xB1, 0x45, 0x4D, 0xC2, 0x69, 0x7D, 0x20
    };
#else
    static uint8_t EXPECTED_RESULT_SHA256[] = {
        0xB5, 0xB7, 0x56, 0xD2, 0x6F, 0x8C, 0xDF, 0x6B,
        0xA3, 0xCC, 0xB8, 0x12, 0x5C, 0xE4, 0x4D, 0x0F,
        0xDD, 0x1C, 0x4C, 0xF1, 0x6E, 0x41, 0x9F, 0xED,
        0x52, 0x79, 0x2E, 0x1A, 0x9C, 0x47, 0xDF, 0x2B
    };
#endif /* INPUT_512 */
    void sha256_test(gpio_t active_gpio)
    {
        uint8_t sha256_result[SHA256_DIGEST_LENGTH];
        sha256_context_t ctx;
        xtimer_sleep(1);
        for(int i=0;i<TEST_ENERGY_ITER;i++) {
            printf("Iteration %i/%i\n", i, TEST_ENERGY_ITER);
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
    }
#endif /* SHA256 */
