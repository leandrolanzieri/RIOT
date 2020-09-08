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
 * @brief       Application to measure energy for sha256
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "hashes/sha256.h"
#include "sha256_hwctx.h"

#include "board.h"
#include "xtimer.h"

#ifndef TEST_STACK
#include "periph/gpio.h"
#else
#include "ps.h"
#endif

static const unsigned char SHA_TESTSTRING[] = "This is a teststring fore sha256";
static size_t SHA_TESTSTR_SIZE = 32;

#ifndef TEST_STACK
    static uint8_t EXPECTED_RESULT_SHA256[] = {
        0x65, 0x0C, 0x3A, 0xC7, 0xF9, 0x33, 0x17, 0xD3,
        0x96, 0x31, 0xD3, 0xF5, 0xC5, 0x5B, 0x0A, 0x1E,
        0x96, 0x68, 0x04, 0xE2, 0x73, 0xC3, 0x8F, 0x93,
        0x9C, 0xB1, 0x45, 0x4D, 0xC2, 0x69, 0x7D, 0x20
    };
#endif
    void sha256_test_energy(gpio_t start, gpio_t stop)
    {
#ifndef TEST_STACK
        // initial state of start pin is high
        gpio_set(start);

        // delay to start current measuremnt tool
        xtimer_sleep(5);

        for (int i=0; i < TEST_ENERGY_ITER; i++)
        {
            // start measurement round
            gpio_clear(start);
#endif
            uint8_t sha256_result[SHA256_DIGEST_LENGTH];
            sha256_context_t ctx;

            sha256_init(&ctx);
            sha256_update(&ctx, SHA_TESTSTRING, SHA_TESTSTR_SIZE);
            sha256_final(&ctx, sha256_result);
#ifndef TEST_STACK
            // end measurement round
            gpio_set(stop);

            // reset I/O pins for next round
            gpio_set(start);
            gpio_clear(stop);

            xtimer_usleep(100 * 1000);

            if (memcmp(sha256_result, EXPECTED_RESULT_SHA256, SHA256_DIGEST_LENGTH) != 0) {
                LED0_ON;
            }
        }
#else
        // ps();
        printf("sizeof(ctx): %i\n", sizeof(ctx));
        (void)start;
        (void)stop;
#endif
        puts("DONE");
    }
