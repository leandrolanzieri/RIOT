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
 * @brief       Application to measure and compare crypto runtime
 *
 * @author      Lena Boeckmann <lena.boeckmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef ARM_CRYPTOCELL
#include "armcc_setup.h"
#endif

#include "crypto_runtime.h"

#include "periph/gpio.h"
#include "periph_conf.h"

#include "crypto/aes.h"
#include "crypto/ciphers.h"

#ifdef BOARD_PBA_D_01_KW2X
    gpio_t active_gpio = GPIO_PIN(2,5);
#endif /*MODULE_LIB_MMCAU*/

// /* AES Test */
// static uint8_t KEY[] = {
//     0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
//     0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
// };
// static uint8_t KEY_LEN = 16;

// static uint8_t PLAIN[] = {
//     0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
//     0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
//     0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
//     0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
//     0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
//     0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
//     0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
//     0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10
// };
// static uint8_t PLAIN_LEN = 64;

// static uint8_t ECB_CIPHER[] = {
//     0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60,
//     0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97,
//     0xf5, 0xd3, 0xd5, 0x85, 0x03, 0xb9, 0x69, 0x9d,
//     0xe7, 0x85, 0x89, 0x5a, 0x96, 0xfd, 0xba, 0xaf,
//     0x43, 0xb1, 0xcd, 0x7f, 0x59, 0x8e, 0xce, 0x23,
//     0x88, 0x1b, 0x00, 0xe3, 0xed, 0x03, 0x06, 0x88,
//     0x7b, 0x0c, 0x78, 0x5e, 0x27, 0xe8, 0xad, 0x3f,
//     0x82, 0x23, 0x20, 0x71, 0x04, 0x72, 0x5d, 0xd4
// };
// static uint8_t ECB_CIPHER_LEN = 64;

// void aes_ecb_test(gpio_t active_gpio)
// {
//     int ret;
//     cipher_context_t ctx;
//     uint8_t data[ECB_CIPHER_LEN];
//     memset(data, 0, ECB_CIPHER_LEN);

//     gpio_set(active_gpio);
//     ret = aes_init(&ctx, KEY, KEY_LEN);
//     gpio_clear(active_gpio);
//     if (ret < 1) {
//         printf("AES Init failed: %d\n", ret);
//     }

//     gpio_set(active_gpio);
//     ret = aes_encrypt_ecb(&ctx, PLAIN, PLAIN_LEN, data);
//     gpio_clear(active_gpio);
//     if (ret < 0) {
//         printf("AES ECB Enryption failed: %d\n", ret);
//         return;
//     }

//     if (memcmp(data, ECB_CIPHER, ECB_CIPHER_LEN)) {
//         printf("AES ECB encryption wrong cipher\n");
//         return;
//     }

//     memset(data, 0, ECB_CIPHER_LEN);

//     gpio_set(active_gpio);
//     ret = aes_decrypt_ecb(&ctx, PLAIN, PLAIN_LEN, data);
//     gpio_clear(active_gpio);

//     if (ret < 0) {
//         printf("AES ECB Deryption failed: %d\n", ret);
//         return;
//     }

//     if (memcmp(data, PLAIN, PLAIN_LEN)) {
//         printf("AES ECB decryption wrong cipher\n");
//         return;
//     }

//     printf("AES CTR encrypt/decrypt successful\n");
// }

int main(void)
{
    // printf("Decryption fine\n");
    // printf("AES CTR encrypt/decrypt successful\n");
    // sha1_test(active_gpio);
    // sha256_test(active_gpio);
    // aes_cbc_test(active_gpio);
    aes_ecb_test(active_gpio);
    // aes_ctr_test(active_gpio);
    // hmac_sha256_test(active_gpio);

    return 1;
}