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

#include "board.h"
#include "periph/gpio.h"

#if !defined(TEST_STACK) && !defined(TEST_MEM)
#include "xtimer.h"
#else
#include "ps.h"
#endif

#ifdef INPUT_512
    static unsigned char SHA_TESTSTRING[] = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Ste";
    #define SHA_TESTSTR_SIZE 512
#else
    static const unsigned char SHA_TESTSTRING[] = "This is a teststring fore sha256";
    #define SHA_TESTSTR_SIZE 32
#endif

#if !defined(TEST_STACK) && !defined(TEST_MEM)
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
#endif
extern gpio_t gpio_sync_pin;
    void sha256_test_energy(gpio_t start, gpio_t stop)
    {
#if !defined(TEST_STACK) && !defined(TEST_MEM)
        // initial state of start pin is high
        gpio_set(start);
        gpio_init(gpio_sync_pin, GPIO_IN);

        // delay to start current measuremnt tool
        // xtimer_sleep(5);

        for (int i=0; i < TEST_ENERGY_ITER; i++)
        {
            while(gpio_read(gpio_sync_pin)) {};
            while(!gpio_read(gpio_sync_pin)) {};

            // start measurement round
            gpio_clear(start);
#endif
            uint8_t sha256_result[SHA256_DIGEST_LENGTH];
            sha256_context_t ctx;

            sha256_init(&ctx);
            sha256_update(&ctx, SHA_TESTSTRING, SHA_TESTSTR_SIZE);
            sha256_final(&ctx, sha256_result);
#if !defined(TEST_STACK) && !defined(TEST_MEM)
            // end measurement round
            gpio_set(stop);

            // reset I/O pins for next round
            gpio_set(start);
            gpio_clear(stop);

            // xtimer_usleep(100 * 1000);
            // xtimer_sleep(1);

            if (memcmp(sha256_result, EXPECTED_RESULT_SHA256, SHA256_DIGEST_LENGTH) != 0) {
                LED0_ON;
            }
        }
#endif
#ifdef TEST_STACK
        ps();
        printf("sizeof(ctx): %i\n", sizeof(ctx));
#endif
        (void)start;
        (void)stop;
        puts("DONE");
    }
