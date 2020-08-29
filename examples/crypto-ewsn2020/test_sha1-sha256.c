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

#ifdef TIME_TEST
    #include "xtimer.h"

     /* Timer variables */
    static uint32_t start, stop, t_diff;
#endif /* TIME_TEST */

static char SHA_TESTSTRING[] = "This is a teststring to compare SHA-Algorithms with and without hardware acceleration";

static uint8_t EXPECTED_RESUL_SHA1[] = {
    0x76, 0xe7, 0x93, 0x20, 0x02,
    0x13, 0xbf, 0xb1, 0x9b, 0xfa,
    0x5e, 0xcb, 0xcf, 0x84, 0x7a,
    0x6d, 0x19, 0x7a, 0x68, 0x6b
};

static uint8_t EXPECTED_RESULT_SHA256[] = {
    0x42, 0x08, 0x94, 0x3c, 0x39, 0x1a, 0x44, 0x68,
    0xff, 0x5f, 0x71, 0x21, 0x08, 0xca, 0xc1, 0xfd,
    0x61, 0xf5, 0xf4, 0x79, 0xf8, 0x27, 0x4f, 0x0e,
    0xa6, 0xcd, 0xce, 0xd5, 0x90, 0x61, 0x28, 0x29
};

static size_t SHA_TESTSTR_SIZE = (sizeof(SHA_TESTSTRING)-1);

void sha1_test(gpio_t active_gpio)
{
    uint8_t sha1_result[SHA1_DIGEST_LENGTH];
    sha1_context ctx;

#ifdef TIME_TEST
    start = xtimer_now_usec();
    sha1(sha1_result, (unsigned char*)SHA_TESTSTRING, SHA_TESTSTR_SIZE);
    stop = xtimer_now_usec();
    t_diff = stop - start;
    printf("Sha1 Time: %ld us\n", t_diff);
#else
    gpio_set(active_gpio);
    sha1_init(&ctx);
    gpio_clear(active_gpio);

    gpio_set(active_gpio);
    sha1_update(&ctx, SHA_TESTSTRING, SHA_TESTSTR_SIZE);
    gpio_clear(active_gpio);

    gpio_set(active_gpio);
    sha1_final(&ctx, sha1_result);
    gpio_clear(active_gpio);
#endif /* TIME_TEST */

    if (memcmp(sha1_result, EXPECTED_RESUL_SHA1, SHA1_DIGEST_LENGTH) != 0) {
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

void sha256_test(gpio_t active_gpio)
{
    uint8_t sha256_result[SHA256_DIGEST_LENGTH];
    sha256_context_t ctx;

#ifdef TIME_TEST
    start = xtimer_now_usec();
    sha256((unsigned char*)SHA_TESTSTRING, SHA_TESTSTR_SIZE, sha256_result);
    stop = xtimer_now_usec();
    t_diff = stop - start;
    printf("Sha256 Time: %ld us\n", t_diff);
#else
    gpio_set(active_gpio);
    sha256_init(&ctx);
    gpio_clear(active_gpio);

    gpio_set(active_gpio);
    sha256_update(&ctx, SHA_TESTSTRING, SHA_TESTSTR_SIZE);
    gpio_clear(active_gpio);

    gpio_set(active_gpio);
    sha256_final(&ctx, sha256_result);
    gpio_clear(active_gpio);
#endif /* TIME_TEST */

    if (memcmp(sha256_result, EXPECTED_RESULT_SHA256, SHA256_DIGEST_LENGTH) != 0) {
        printf("SHA-256 Failure\n");

        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            printf("%02x ", sha256_result[i]);
        }
        printf("\n");
    }
    else {
        printf("SHA-256 Success\n");
    }
}
