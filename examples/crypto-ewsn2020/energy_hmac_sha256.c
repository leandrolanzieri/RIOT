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
 * @brief       Application to measure energy for hmac sha256
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

    static char HMAC_INPUT[] = {
        0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20,
        0x61, 0x20, 0x74, 0x65, 0x73, 0x74, 0x73, 0x74,
        0x72, 0x69, 0x6e, 0x67, 0x20, 0x66, 0x6f, 0x72,
        0x20, 0x68, 0x6d, 0x61, 0x63, 0x32, 0x35, 0x36
    };
    size_t HMAC_INPUT_SIZE = 32;

    static uint8_t HMAC_KEY[] = {
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b
    };
    size_t KEY_SIZE = 64;
#ifndef TEST_STACK
    static uint8_t EXPECTED_RESULT[] = {
        0x39, 0x38, 0x66, 0x75, 0xdc, 0xc9, 0xe1, 0x86,
        0x58, 0xac, 0xfe, 0x34, 0x05, 0x79, 0xe5, 0x1b,
        0x20, 0x02, 0x8d, 0xc6, 0x3c, 0x70, 0xaf, 0x80,
        0xe5, 0x2d, 0xe4, 0x22, 0x7a, 0x41, 0x0c, 0x70
    };
#endif
    void hmac_sha256_test_energy(gpio_t start, gpio_t stop)
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
            uint8_t hmac_result[SHA256_DIGEST_LENGTH];
            hmac_context_t ctx;

            hmac_sha256_init(&ctx, HMAC_KEY, KEY_SIZE);
            hmac_sha256_update(&ctx, HMAC_INPUT, HMAC_INPUT_SIZE );
            hmac_sha256_final(&ctx, hmac_result);
#ifndef TEST_STACK
            // end measurement round
            gpio_set(stop);

            // reset I/O pins for next round
            gpio_set(start);
            gpio_clear(stop);

            xtimer_usleep(200 * 1000);

            if (memcmp(hmac_result, EXPECTED_RESULT, SHA256_DIGEST_LENGTH) != 0) {
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
