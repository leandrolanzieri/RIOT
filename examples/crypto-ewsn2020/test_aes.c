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

#include "crypto/aes.h"
#include "crypto/ciphers.h"

#include "periph/gpio.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

/* AES Test */
#if AES_CBC || AES_CTR || AES_ECB
static uint8_t KEY[] = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
static uint8_t KEY_LEN = 16;
#endif

#ifdef AES_CBC

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

    void aes_cbc_test(gpio_t active_gpio)
    {
        (void) active_gpio;

        int ret;
        cipher_context_t enc, dec;
        uint8_t data[CBC_CIPHER_LEN];
        memset(data, 0, CBC_CIPHER_LEN);

        gpio_set(active_gpio);
        ret = aes_init(&enc, KEY, KEY_LEN);
        gpio_clear(active_gpio);
        if (ret < 1) {
            printf("AES CBC Enc Init failed: %d\n", ret);
            return;
        }

        gpio_set(active_gpio);
        ret = aes_encrypt_cbc(&enc, CBC_IV, CBC_PLAIN, CBC_PLAIN_LEN, data);
        gpio_clear(active_gpio);

        if (ret < 0) {
            printf("AES CBC Encrypt failed: %d\n", ret);
            return;
        }

        if (memcmp(data, CBC_CIPHER, CBC_CIPHER_LEN)) {
            printf("AES CBC encryption wrong cipher\n");
            // return;
        }

        memset(data, 0, CBC_CIPHER_LEN);
        gpio_set(active_gpio);
        ret = aes_init(&dec, KEY, KEY_LEN);
        gpio_clear(active_gpio);
        if (ret < 1) {
            printf("AES CBC Dec Init failed: %d\n", ret);
            return;
        }

        gpio_set(active_gpio);
        ret = aes_decrypt_cbc(&dec, CBC_IV, CBC_CIPHER, CBC_CIPHER_LEN, data);
        gpio_clear(active_gpio);

        if (ret < 0) {
            printf("AES CBC Decrypt failed: %d\n", ret);
            return;
        }

        if (memcmp(data, CBC_PLAIN, CBC_CIPHER_LEN)) {
            printf("AES CBC decryption wrong plain text\n");
            return;
        }
        printf("AES CBC encrypt/decrypt successful\n");
    }

#endif /* AES_CBC */

#ifdef AES_CTR
    static uint8_t CTR_COUNTER[16] = {
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
        0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
    };
    static uint8_t CTR_COUNTER_LEN = 16;

    static uint8_t __attribute__((aligned)) CTR_PLAIN[] = {
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
        0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
        0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51
    };
    static uint8_t CTR_PLAIN_LEN = 32;

    static uint8_t CTR_CIPHER[] = {
        0x87, 0x4d, 0x61, 0x91, 0xb6, 0x20, 0xe3, 0x26,
        0x1b, 0xef, 0x68, 0x64, 0x99, 0x0d, 0xb6, 0xce,
        0x98, 0x06, 0xf6, 0x6b, 0x79, 0x70, 0xfd, 0xff,
        0x86, 0x17, 0x18, 0x7b, 0xb9, 0xff, 0xfd, 0xff
    };
    static uint8_t CTR_CIPHER_LEN = 32;

    void aes_ctr_test(gpio_t active_gpio)
    {
        int ret;
        cipher_context_t ctx;
        uint8_t ctr[CTR_COUNTER_LEN];
        uint8_t data[CTR_CIPHER_LEN];
        memset(data, 0, CTR_CIPHER_LEN);
        memcpy(ctr, CTR_COUNTER, CTR_COUNTER_LEN);

        gpio_set(active_gpio);
        ret = aes_init(&ctx, KEY, KEY_LEN);
        gpio_clear(active_gpio);
        if (ret < 1) {
            printf("AES CTR Init failed: %d\n", ret);
            return;
        }

        gpio_set(active_gpio);
        ret = aes_encrypt_ctr(&ctx, ctr, 0, CTR_PLAIN, CTR_PLAIN_LEN, data);
        gpio_clear(active_gpio);

        if (ret < 0) {
            printf("AES CTR Enryption failed: %d\n", ret);
            return;
        }

        if (memcmp(data, CTR_CIPHER, CTR_CIPHER_LEN)) {
            printf("AES CTR encryption wrong cipher\n");
            return;
        }

        memset(data, 0, CTR_CIPHER_LEN);
        memcpy(ctr, CTR_COUNTER, CTR_COUNTER_LEN);

        gpio_set(active_gpio);
        ret = aes_decrypt_ctr(&ctx, ctr, 0, CTR_CIPHER, CTR_CIPHER_LEN, data);
        gpio_clear(active_gpio);

        if (ret < 0) {
            printf("AES CTR Decryption failed: %d\n", ret);
            return;
        }
        if (memcmp(data, CTR_PLAIN, CBC_CIPHER_LEN)) {
            printf("AES CTR decryption wrong plain text\n");
            return;
        }
        printf("AES CTR encrypt/decrypt successful\n");
    }
#endif /* AES_CTR */

#ifdef AES_ECB
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

    void aes_ecb_test(gpio_t active_gpio)
    {
        int ret;
        cipher_context_t ctx;
        uint8_t data[ECB_CIPHER_LEN];
        memset(data, 0, ECB_CIPHER_LEN);

        gpio_set(active_gpio);
        ret = aes_init(&ctx, KEY, KEY_LEN);
        gpio_clear(active_gpio);
        if (ret < 1) {
            printf("AES Init failed: %d\n", ret);
        }

        gpio_set(active_gpio);
        ret = aes_encrypt_ecb(&ctx, ECB_PLAIN, ECB_PLAIN_LEN, data);
        gpio_clear(active_gpio);
        if (ret < 0) {
            printf("AES ECB Enryption failed: %d\n", ret);
            return;
        }

        memset(data, 0, ECB_CIPHER_LEN);

        gpio_set(active_gpio);
        ret = aes_decrypt_ecb(&ctx, ECB_CIPHER, ECB_CIPHER_LEN, data);
        gpio_clear(active_gpio);

        if (ret < 0) {
            printf("AES ECB Deryption failed: %d\n", ret);
            return;
        }

        if (memcmp(data, ECB_PLAIN, ECB_PLAIN_LEN)) {
            printf("AES ECB decryption wrong cipher\n");
            return;
        }

        printf("AES ECB encrypt/decrypt successful\n");
    }
#endif /* AES_ECB */