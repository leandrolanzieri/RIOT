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
 * @brief       Application to measure AES CTR and CBC energy
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "xtimer.h"
#include "crypto/aes.h"
#include "crypto/ciphers.h"

#include "periph/gpio.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

static uint8_t KEY[] = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
static uint8_t KEY_LEN = 16;

void exp_frame(gpio_t start, gpio_t stop, uint8_t *data, uint8_t *data_exp, size_t len_exp);

#if TEST_ENERGY_AES_CBC
static uint8_t CBC_IV[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

static uint8_t __attribute__((aligned)) CBC_PLAIN[] = {
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
    0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
    0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
    0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51
};
static uint8_t CBC_PLAIN_LEN = 32;

static uint8_t __attribute__((aligned)) CBC_CIPHER[] = {
    0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46,
    0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d,
    0x50, 0x86, 0xcb, 0x9b, 0x50, 0x72, 0x19, 0xee,
    0x95, 0xdb, 0x11, 0x3a, 0x91, 0x76, 0x78, 0xb2
};
static uint8_t CBC_CIPHER_LEN = 32;


void aes_cbc_enc_test_energy(gpio_t start, gpio_t stop)
{
    uint8_t data[CBC_CIPHER_LEN];
    exp_frame(start, stop, data, CBC_CIPHER, CBC_CIPHER_LEN);
}

void aes_cbc_dec_test_energy(gpio_t start, gpio_t stop)
{
    uint8_t data[CBC_CIPHER_LEN];
    exp_frame(start, stop, data, CBC_PLAIN, CBC_PLAIN_LEN);
}

#endif /* TEST_ENERGY_AES_CBC */

#if TEST_ENERGY_AES_ECB
static uint8_t __attribute__((aligned)) ECB_PLAIN[] = {
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
    0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a
};
static uint8_t ECB_PLAIN_LEN = 16;

static uint8_t __attribute__((aligned))ECB_CIPHER[] = {
    0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60,
    0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97
};
static uint8_t ECB_CIPHER_LEN = 16;

void aes_ecb_enc_test_energy(gpio_t start, gpio_t stop)
{
    uint8_t data[ECB_CIPHER_LEN];
    exp_frame(start, stop, data, ECB_CIPHER, ECB_CIPHER_LEN);
}

void aes_ecb_dec_test_energy(gpio_t start, gpio_t stop)
{
    uint8_t data[ECB_PLAIN_LEN];
    exp_frame(start, stop, data, ECB_PLAIN, ECB_PLAIN_LEN);
}

#endif /* TEST_ENERGY_AES_ECB */

void exp_frame(gpio_t start, gpio_t stop, uint8_t *data, uint8_t *data_exp, size_t len_exp)
{
    // initial state of start pin is high
    gpio_set(start);

    // delay to start current measuremnt tool
    xtimer_sleep(5);

    for (int i=0; i < TEST_ENERGY_ITER; i++)
    {
        // start measurement round
        gpio_clear(start);

        cipher_context_t ctx;

#if TEST_ENERGY_AES_CBC_ENC
        aes_init(&ctx, KEY, KEY_LEN);
        aes_encrypt_cbc(&ctx, CBC_IV, CBC_PLAIN, CBC_PLAIN_LEN, data);
#elif TEST_ENERGY_AES_CBC_DEC
        aes_init(&ctx, KEY, KEY_LEN);
        aes_decrypt_cbc(&ctx, CBC_IV, CBC_CIPHER, CBC_CIPHER_LEN, data);
#elif TEST_ENERGY_AES_ECB_ENC
        aes_init(&ctx, KEY, KEY_LEN);
        aes_encrypt_ecb(&ctx, ECB_PLAIN, ECB_PLAIN_LEN, data);
#elif TEST_ENERGY_AES_ECB_DEC
        aes_init(&ctx, KEY, KEY_LEN);
        aes_decrypt_ecb(&ctx, ECB_CIPHER, ECB_CIPHER_LEN, data);
#endif
        // end measurement round
        gpio_set(stop);

        // reset I/O pins for next round
        gpio_set(start);
        gpio_clear(stop);

        xtimer_usleep(100 * 1000);

        if (memcmp(data, data_exp, len_exp)) {
            LED0_ON;
    }
    puts("DONE");
}

}
