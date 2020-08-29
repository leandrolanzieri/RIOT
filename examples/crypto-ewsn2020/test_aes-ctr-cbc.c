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
 * @brief       Application to measure AES CTR and CBC runtime
 *
 * @author      Lena Boeckmann <lena.boeckmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "crypto/modes/aes_ctr.h"
#include "crypto/modes/aes_cbc.h"
#include "crypto/ciphers.h"

#include "periph/gpio.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#ifdef TIME_TEST
    #include "xtimer.h"

    /* Timer variables */
    static uint32_t start, stop, t_diff;
#endif /* TIME_TEST */

/* AES Test */
static uint8_t KEY[] = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
static uint8_t KEY_LEN = 16;

static uint8_t CBC_IV[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

static uint8_t CTR_COUNTER[16] = {
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
    0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};
static uint8_t CTR_COUNTER_LEN = 16;

static uint8_t CBC_CTR_PLAIN[] = {
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
    0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
    0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
    0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
    0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
    0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
    0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
    0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10
};
static uint8_t PLAIN_LEN = 64;

static uint8_t CBC_CIPHER[] = {
    0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46,
    0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d,
    0x50, 0x86, 0xcb, 0x9b, 0x50, 0x72, 0x19, 0xee,
    0x95, 0xdb, 0x11, 0x3a, 0x91, 0x76, 0x78, 0xb2,
    0x73, 0xbe, 0xd6, 0xb8, 0xe3, 0xc1, 0x74, 0x3b,
    0x71, 0x16, 0xe6, 0x9e, 0x22, 0x22, 0x95, 0x16,
    0x3f, 0xf1, 0xca, 0xa1, 0x68, 0x1f, 0xac, 0x09,
    0x12, 0x0e, 0xca, 0x30, 0x75, 0x86, 0xe1, 0xa7
};
static uint8_t CBC_CIPHER_LEN = 64;

static uint8_t CTR_CIPHER[] = {
    0x87, 0x4d, 0x61, 0x91, 0xb6, 0x20, 0xe3, 0x26,
    0x1b, 0xef, 0x68, 0x64, 0x99, 0x0d, 0xb6, 0xce,
    0x98, 0x06, 0xf6, 0x6b, 0x79, 0x70, 0xfd, 0xff,
    0x86, 0x17, 0x18, 0x7b, 0xb9, 0xff, 0xfd, 0xff,
    0x5a, 0xe4, 0xdf, 0x3e, 0xdb, 0xd5, 0xd3, 0x5e,
    0x5b, 0x4f, 0x09, 0x02, 0x0d, 0xb0, 0x3e, 0xab,
    0x1e, 0x03, 0x1d, 0xda, 0x2f, 0xbe, 0x03, 0xd1,
    0x79, 0x21, 0x70, 0xa0, 0xf3, 0x00, 0x9c, 0xee
};
static uint8_t CTR_CIPHER_LEN = 64;

static int aes_cbc_enc_test(cipher_context_t* ctx, uint8_t* data, gpio_t active_gpio)
{
    int err;
#ifdef TIME_TEST
    start = xtimer_now_usec();
    err = aes_encrypt_cbc(&ctx, CBC_IV, CBC_CTR_PLAIN, PLAIN_LEN, data);
    stop = xtimer_now_usec();
    t_diff = stop - start;
    printf("AES Encrypt time: %ld us\n", t_diff);
#else
    gpio_set(active_gpio);
    err = aes_encrypt_cbc(ctx, CBC_IV, CBC_CTR_PLAIN, PLAIN_LEN, data);
    gpio_clear(active_gpio);
#endif /* TIME_TEST */

    if (err < 0) {
        DEBUG("AES CBC Encrypt failed: %d\n", err);
        return err;
    }

    if (memcmp(data, CBC_CIPHER, CBC_CIPHER_LEN)) {
        printf("AES CBC encryption wrong cipher\n");
        return -1;
    }
    return 0;
}

static int aes_cbc_dec_test(cipher_context_t* ctx, uint8_t* data, gpio_t active_gpio)
{
    int err;
    #ifdef TIME_TEST
    start = xtimer_now_usec();
    err = aes_decrypt_cbc(&ctx, CBC_IV, CBC_CIPHER, CBC_CIPHER_LEN, data);
    stop = xtimer_now_usec();
    t_diff = stop - start;
    printf("AES CBC Decrypt time: %ld us\n", t_diff);
#else
    gpio_set(active_gpio);
    err = aes_decrypt_cbc(ctx, CBC_IV, CBC_CIPHER, CBC_CIPHER_LEN, data);
    gpio_clear(active_gpio);
#endif /* TIME_TEST */

    if (err < 0) {
        printf("AES CBC Decrypt failed: %d\n", err);
        return err;
    }

    if (memcmp(data, CBC_CTR_PLAIN, CBC_CIPHER_LEN)) {
        printf("AES CBC decryption wrong plain text\n");
        return -1;
    }
    return 0;
}

static int aes_ctr_enc_test(cipher_context_t* ctx, uint8_t* data, uint8_t* ctr, gpio_t active_gpio)
{
    int err;
#ifdef TIME_TEST
    start = xtimer_now_usec();
    err = aes_encrypt_ctr(ctx, ctr, 0, CBC_CTR_PLAIN, PLAIN_LEN, data);
    stop = xtimer_now_usec();
    t_diff = stop - start;
    printf("AES CTR Encrypt time: %ld us\n", t_diff);
#else
    gpio_set(active_gpio);
    err = aes_encrypt_ctr(ctx, ctr, 0, CBC_CTR_PLAIN, PLAIN_LEN, data);
    gpio_clear(active_gpio);
#endif /* TIME_TEST */

    if (err < 0) {
        printf("AES CTR Enryption failed: %d\n", err);
        return err;
    }

    if (memcmp(data, CTR_CIPHER, CTR_CIPHER_LEN)) {
        printf("AES CTR encryption wrong cipher\n");
        return -1;
    }
    return 0;
}

static int aes_ctr_dec_test(cipher_context_t* ctx, uint8_t* data, uint8_t* ctr, gpio_t active_gpio)
{
    int err;
#ifdef TIME_TEST
    start = xtimer_now_usec();
    err = aes_decrypt_ctr(ctx, ctr, 0, CTR_CIPHER, CTR_CIPHER_LEN, data);
    stop = xtimer_now_usec();
    t_diff = stop - start;
    printf("AES CTR Decrypt time: %ld us\n", t_diff);
#else
    gpio_set(active_gpio);
    err = aes_decrypt_ctr(ctx, ctr, 0, CTR_CIPHER, CTR_CIPHER_LEN, data);
    gpio_clear(active_gpio);
#endif /* TIME_TEST */
    if (err < 0) {
        printf("AES CTR Decryption failed: %d\n", err);
        return err;
    }
    if (memcmp(data, CBC_CTR_PLAIN, CBC_CIPHER_LEN)) {
        printf("AES CTR decryption wrong plain text\n");
        return -1;
    }
    return 0;
}

void aes_cbc_test(gpio_t active_gpio)
{
    int err;
    cipher_context_t c_ctx;
    uint8_t data[CBC_CIPHER_LEN];
    memset(data, 0, CBC_CIPHER_LEN);

    gpio_set(active_gpio);
    err = cipher_init(&c_ctx, CIPHER_AES_128, KEY, KEY_LEN);
    gpio_clear(active_gpio);
    if (err < 1) {
        printf("AES CBC Init failed: %d\n", err);
        return;
    }

    err = aes_cbc_enc_test(&c_ctx, data, active_gpio);
    if (err < 0) {
        printf("AES CBC Encryption failed: %d\n", err);
        return;
    }

    memset(data, 0, CBC_CIPHER_LEN);

    err = aes_cbc_dec_test(&c_ctx, data, active_gpio);
    if (err < 0) {
        printf("AES CBC Decryption failed: %d\n", err);
        return;
    }
    printf("AES CBC encrypt/decrypt successful\n");
}

void aes_ctr_test(gpio_t active_gpio)
{
    int err;
    cipher_context_t c_ctx;
    uint8_t ctr[CTR_COUNTER_LEN];
    uint8_t data[CTR_CIPHER_LEN];
    memset(data, 0, CTR_CIPHER_LEN);
    memcpy(ctr, CTR_COUNTER, CTR_COUNTER_LEN);

    gpio_set(active_gpio);
    err = cipher_init(&c_ctx, CIPHER_AES_128, KEY, KEY_LEN);
    gpio_clear(active_gpio);
    if (err < 1) {
        printf("AES CTR Init failed: %d\n", err);
        return;
    }
    err = aes_ctr_enc_test(&c_ctx, data, ctr, active_gpio);
    if (err < 0) {
        printf("AES CTR Encryption failed: %d\n", err);
        return;
    }
    memset(data, 0, CTR_CIPHER_LEN);
    memcpy(ctr, CTR_COUNTER, CTR_COUNTER_LEN);

    err = aes_ctr_enc_test(&c_ctx, data, ctr, active_gpio);
    if (err < 0) {
        printf("AES CTR Decryption failed: %d\n", err);
        return;
    }
    printf("AES CTR encrypt/decrypt successful\n");
}
