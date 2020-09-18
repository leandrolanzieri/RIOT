/*
 * Copyright (C) 2008, 2009, 2010 Kaspar Schleiser <kaspar@schleiser.de>
 * Copyright (C) 2013 INRIA
 * Copyright (C) 2013 Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
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
 * @brief       Default application that shows a lot of functionality of RIOT
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "hashes/sha256.h"
#include "sha256_hwctx.h"
#include "thread.h"
#include "periph/gpio.h"

gpio_t active_gpio = GPIO_PIN(2, 6);

char stack_01[THREAD_STACKSIZE_MAIN];
char stack_02[THREAD_STACKSIZE_MAIN];

static uint8_t EXPECTED_RESULT_SHA256[] = {
        0x65, 0x0C, 0x3A, 0xC7, 0xF9, 0x33, 0x17, 0xD3,
        0x96, 0x31, 0xD3, 0xF5, 0xC5, 0x5B, 0x0A, 0x1E,
        0x96, 0x68, 0x04, 0xE2, 0x73, 0xC3, 0x8F, 0x93,
        0x9C, 0xB1, 0x45, 0x4D, 0xC2, 0x69, 0x7D, 0x20
};
static const unsigned char SHA_TESTSTRING[] = "This is a teststring fore sha256";
static size_t SHA_TESTSTR_SIZE = 32;

void* crypto_thread(void *arg){

    (void) arg;
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
    return NULL;
}

int main(void)
{
    (void) puts("Welcome to RIOT!");
    gpio_init(active_gpio, GPIO_OUT);
    gpio_clear(active_gpio);
    // pthread_t thread_01;
    // pthread_t thread_02;

    // if (pthread_create(&thread_01, NULL, crypto_thread, NULL)) {
    //     printf("Failed creating thread 1\n");
    // }

    // if (pthread_create(&thread_02, NULL, crypto_thread, NULL)) {
    //     printf("Failed creating thread 1\n");
    // }

    thread_create(stack_01, sizeof(stack_01), THREAD_PRIORITY_MAIN - 1,
            THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST, crypto_thread, NULL, "thread_01");
    thread_create(stack_02, sizeof(stack_01), THREAD_PRIORITY_MAIN - 1,
            THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST, crypto_thread, NULL, "thread_02");
    return 0;
}
