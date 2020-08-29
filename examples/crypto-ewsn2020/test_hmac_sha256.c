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

#ifdef TIME_TEST
    #include "xtimer.h"

    /* Timer variables */
    static uint32_t start, stop, t_diff;
#endif /* TIME_TEST */

static const unsigned char HMAC_TESTSTR[] = "This is a teststring to measure HMAC-SHA256 runtime with and without hardware acceleration";
static size_t HMAC_TESTSTR_SIZE = (sizeof(HMAC_TESTSTR)-1);

static unsigned char HMAC_KEY[] = { 'k', 'e', 'y' };
static uint8_t HMAC_KEY_LEN = 3;

static uint8_t EXPECTED_RESULT[] = {
    0x33, 0x30, 0xb6, 0xd9, 0xf1, 0xb0, 0x34, 0x69,
    0xd9, 0x4e, 0xd8, 0xe7, 0x90, 0xda, 0x56, 0x32,
    0xe1, 0x05, 0x38, 0x8f, 0xe1, 0xbc, 0x7d, 0xef,
    0xe3, 0xe2, 0xa6, 0x13, 0x03, 0x08, 0xb8, 0xf7
};

void hmac_sha256_test(gpio_t active_gpio)
{
    uint8_t hmac_result[SHA256_DIGEST_LENGTH];
    hmac_context_t ctx;

#ifdef TIME_TEST
    start = xtimer_now_usec();
    hmac_sha256(HMAC_KEY, HMAC_KEY_LEN, HMAC_TESTSTR, HMAC_TESTSTR_SIZE, hmac_result);
    stop = xtimer_now_usec();
    t_diff = stop - start;
    printf("HMAC Sha256 Time: %ld us\n", t_diff);
#else
    gpio_set(active_gpio);
    hmac_sha256_init(&ctx, HMAC_KEY, HMAC_KEY_LEN);
    gpio_clear(active_gpio);

    gpio_set(active_gpio);
    hmac_sha256_update(&ctx, HMAC_TESTSTR, HMAC_TESTSTR_SIZE);
    gpio_clear(active_gpio);

    gpio_set(active_gpio);
    hmac_sha256_final(&ctx, hmac_result);
    gpio_clear(active_gpio);
#endif /* TIME_TEST */

    if (memcmp(hmac_result, EXPECTED_RESULT, SHA256_DIGEST_LENGTH)) {
        printf("HMAC SHA-256 Failure\n");
        return;
    }
    printf("HMAC SHA-256 Success\n");
}