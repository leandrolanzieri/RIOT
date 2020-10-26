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

#include "board.h"
#include "periph/gpio.h"

#if (!defined(TEST_STACK) && !defined(TEST_MEM)) || defined(USE_XTIMER)
#include "xtimer.h"
#else
#include "ps.h"
#endif

#ifdef INPUT_512
    static unsigned char HMAC_INPUT[] = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Ste";
    #define HMAC_INPUT_SIZE 512
#else
    static char HMAC_INPUT[] = {
        0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20,
        0x61, 0x20, 0x74, 0x65, 0x73, 0x74, 0x73, 0x74,
        0x72, 0x69, 0x6e, 0x67, 0x20, 0x66, 0x6f, 0x72,
        0x20, 0x68, 0x6d, 0x61, 0x63, 0x32, 0x35, 0x36
    };
    #define HMAC_INPUT_SIZE 32
#endif /* INPUT_512 */

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
#define KEY_SIZE 64

#if !defined(TEST_STACK) && !defined(TEST_MEM)
#ifdef INPUT_512
    static uint8_t EXPECTED_RESULT[] = {
        0xa1, 0x7f, 0x58, 0x90, 0xf8, 0x76, 0xd1, 0xb5,
        0xb4, 0xaa, 0x5e, 0xf2, 0x8f, 0xad, 0xd4, 0x55,
        0x24, 0xb0, 0x69, 0xe2, 0x42, 0x07, 0x6a, 0x91,
        0x1b, 0x73, 0x57, 0xe5, 0x6e, 0xff, 0x20, 0x51
    };
#else
    static uint8_t EXPECTED_RESULT[] = {
        0x39, 0x38, 0x66, 0x75, 0xdc, 0xc9, 0xe1, 0x86,
        0x58, 0xac, 0xfe, 0x34, 0x05, 0x79, 0xe5, 0x1b,
        0x20, 0x02, 0x8d, 0xc6, 0x3c, 0x70, 0xaf, 0x80,
        0xe5, 0x2d, 0xe4, 0x22, 0x7a, 0x41, 0x0c, 0x70
    };
#endif /* INPUT_512 */
#endif

#ifndef USE_XTIMER
extern gpio_t gpio_sync_pin;
#endif
    void hmac_sha256_test_energy(gpio_t start, gpio_t stop)
    {
#if !defined(TEST_STACK) && !defined(TEST_MEM)
        // initial state of start pin is high
        gpio_set(start);
#ifndef USE_XTIMER
        gpio_init(gpio_sync_pin, GPIO_IN);
#else
        // delay to start current measuremnt tool
        xtimer_sleep(5);
#endif
        for (int i=0; i < TEST_ENERGY_ITER; i++)
        {
#ifndef USE_XTIMER
            while(gpio_read(gpio_sync_pin)) {};
            while(!gpio_read(gpio_sync_pin)) {};
#endif
            // start measurement round
            gpio_clear(start);
#endif
            uint8_t hmac_result[SHA256_DIGEST_LENGTH];
            hmac_context_t ctx;

            hmac_sha256_init(&ctx, HMAC_KEY, KEY_SIZE);
            hmac_sha256_update(&ctx, HMAC_INPUT, HMAC_INPUT_SIZE );
            hmac_sha256_final(&ctx, hmac_result);
#if !defined(TEST_STACK) && !defined(TEST_MEM)
            // end measurement round
            gpio_set(stop);

            // reset I/O pins for next round
            gpio_set(start);
            gpio_clear(stop);

            // xtimer_usleep(500 * 1000);
            // xtimer_sleep(1);

            if (memcmp(hmac_result, EXPECTED_RESULT, SHA256_DIGEST_LENGTH) != 0) {
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
