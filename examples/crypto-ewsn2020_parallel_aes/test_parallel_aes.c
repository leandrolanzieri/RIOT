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
 * @brief       Parallel AES ECB encryption
 *
 * @author      Lena Boeckmann <lena.boeckmann@haw-hamburg.de>
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "crypto/aes.h"
#include "crypto/ciphers.h"
#include "crypto_util.h"

#include "periph/gpio.h"
#include "xtimer.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

static uint8_t KEY[] = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
static uint8_t KEY_LEN = 16;

#ifndef CONFIG_INPUT_512
    static uint8_t __attribute__((aligned)) ECB_PLAIN[] = {
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a
    };
    static uint8_t ECB_PLAIN_LEN = 32;
#else
    static const unsigned char ECB_PLAIN[] = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Ste";
    static size_t ECB_PLAIN_LEN = 512;
#endif /* CONFIG_INPUT_512 */

#include "thread.h"

#ifndef TEST_AES_ECB_PARALLEL_ITER
#define TEST_AES_ECB_PARALLEL_ITER 1000
#endif

typedef struct {
    gpio_t *pin;
    const char* name;
} thread_info_t;

char aes_ecb_thread_1_stack[THREAD_STACKSIZE_MAIN];
char aes_ecb_thread_2_stack[THREAD_STACKSIZE_MAIN];

void *aes_ecb_thread(void *arg)
{
    thread_info_t *info = (thread_info_t *)arg;
    gpio_t *pin = info->pin;
    int ret;
    cipher_context_t ctx;
    uint8_t data[ECB_PLAIN_LEN];
    memset(data, 0, ECB_PLAIN_LEN);

    aes_init(&ctx, KEY, KEY_LEN);

    for (unsigned i = 0; i < TEST_AES_ECB_PARALLEL_ITER; i++) {
        gpio_set(*pin);
        ret = aes_encrypt_ecb(&ctx, ECB_PLAIN, ECB_PLAIN_LEN, data);
        gpio_clear(*pin);

        if (ret < 0) {
            printf("AES ECB Enryption failed: %d\n", ret);
            return NULL;
        }
    }

    return NULL;
}

thread_info_t info1 = {
    .pin = NULL,
    .name = "Thread 1"
};

thread_info_t info2 = {
    .pin = NULL,
    .name = "Thread 2"
};

void aes_ecb_parallel_test(gpio_t *thread1_pin, gpio_t *thread2_pin)
{
    puts("== Performing parallel test ==");
    printf("=> Encrypting [%d] blocks of [%d] bytes\n", TEST_AES_ECB_PARALLEL_ITER, ECB_PLAIN_LEN);
    if (IS_ACTIVE(CONFIG_EFM32_AES_ECB_NONBLOCKING)) {
        puts("=> Using DMA");
    }
    else {
        puts("=> NOT using DMA");
    }

    gpio_init(*thread1_pin, GPIO_OUT);
    gpio_clear(*thread1_pin);
    gpio_init(*thread2_pin, GPIO_OUT);
    gpio_clear(*thread2_pin);

    info1.pin = thread1_pin;
    info2.pin = thread2_pin;

    thread_create(aes_ecb_thread_1_stack, sizeof(aes_ecb_thread_1_stack),
                  THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
                  aes_ecb_thread, &info1, "AES ECB 1");

    thread_create(aes_ecb_thread_2_stack, sizeof(aes_ecb_thread_2_stack),
                  THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
                  aes_ecb_thread, &info2, "AES ECB 2");
}
