/*
 * Copyright (C) 2014 Freie Universität Berlin
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
 * @brief       Hello World application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef ARM_CRYPTOCELL
#include "armcc_setup.h"
#endif

#include "hashes/sha1.h"
#include "hashes/sha256.h"
#include "crypto/aes.h"
#include "crypto/ciphers.h"
#include "xtimer.h"
#include "periph_conf.h"

/* SHA Tests */
uint8_t sha1_result[SHA1_DIGEST_LENGTH];
uint8_t sha256_result[SHA256_DIGEST_LENGTH];
char teststring[] = "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Aenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Donec quam felis, ultricies nec, pellentesque eu, pretium quis, sem. Nulla consequat massa quis enim. Donec pede justo, fringilla vel, aliquet nec, vulputate eget, arcu. In enim justo, rhoncus ut, imperdiet a, venenatis vitae, justo. Nullam dictum felis eu pede mollis pretium. Integer tincidunt. Cras dapibus. Vivamus elementum semper nisi. Aenean vulputate eleifend tellus. Aenean leo ligula, porttitor eu, consequat vitae, eleifend ac, enim. Aliquam lorem ante, dapibus in, viverra quis, feugiat a, tellus. Phasellus viverra nulla ut metus varius laoreet. Quisque rutrum. Aenean imperdiet. Etiam ultricies nisi vel augue";
uint8_t expected_result_sha1[] = { 0x6a, 0x7c, 0x17, 0x10, 0x4e, 0x56, 0x13, 0xa6, 0x82, 0xc5, 0x2b, 0x84, 0x54, 0xae, 0x5e, 0xbc, 0x9e, 0x8a, 0xf1, 0x67 };

uint8_t expected_result_sha256[] = { 0xfc, 0xbd, 0x7f, 0xe5, 0x12, 0x31, 0x1d, 0x1a, 0x19, 0x33, 0x87, 0x9a, 0x81, 0xe3, 0x42, 0x2e, 0x47, 0x4d, 0xf3, 0xd2, 0x46, 0xdf, 0x82, 0xdf, 0x3f, 0x63, 0x4a, 0xe1, 0x39, 0xd3, 0xb0, 0xa6 };

size_t teststring_size = (sizeof(teststring)-1);

uint8_t teststring2[] = "chili cheese fries";
uint16_t teststring_size2 = (sizeof(teststring2) - 1);
uint8_t expected2[] =
    {0x36, 0x46, 0xEF, 0xD6, 0x27, 0x6C, 0x0D, 0xCB, 0x4B, 0x07, 0x73, 0x41,
     0x88, 0xF4, 0x17, 0xB4, 0x38, 0xAA, 0xCF, 0xC6, 0xAE, 0xEF, 0xFA, 0xBE,
     0xF3, 0xA8, 0x5D, 0x67, 0x42, 0x0D, 0xFE, 0xE5};

/* AES Test */
static uint8_t TEST_0_KEY[] = {
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF
};
static uint8_t TEST_0_INP[] = {
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
};
static uint8_t TEST_0_ENC[] = {
    0x37, 0x29, 0xa3, 0x6c, 0xaf, 0xe9, 0x84, 0xff,
    0x46, 0x22, 0x70, 0x42, 0xee, 0x24, 0x83, 0xf6
};

/* Timer variables */
uint32_t start, stop, t_diff;

static void sha1_test(void)
{
    start = xtimer_now_usec();
    sha1(sha1_result, (unsigned char*)teststring, teststring_size);
    stop = xtimer_now_usec();
    t_diff = stop - start;
    printf("Sha1 Time: %ld us\n", t_diff);

    if (memcmp(sha1_result, expected_result_sha1, SHA1_DIGEST_LENGTH) != 0) {
        printf("SHA-1 Failure\n");

        for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
            printf("%02x ", sha1_result[i]);
        }
        printf("\n");
    }
    else {
        printf("SHA-1 Success\n");
    }
}

static void sha256_test(void)
{
    // start = xtimer_now_usec();
    // sha256((unsigned char*)teststring, teststring_size, sha256_result);
    // stop = xtimer_now_usec();
    // t_diff = stop - start;
    // printf("Sha256 Time: %ld us\n", t_diff);

    sha256_context_t ctx;
    sha256_context_t ctx2;

    sha256_init(&ctx);
    sha256_update(&ctx, (void *)teststring, teststring_size);

    sha256_init(&ctx2);
    sha256_update(&ctx2, (void *)teststring2, teststring_size2);

    sha256_final(&ctx, sha256_result);

    if (memcmp(sha256_result, expected_result_sha256, SHA256_DIGEST_LENGTH) != 0) {
        printf("1 SHA-256 Failure\n");

        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            printf("%02x ", sha256_result[i]);
        }
        printf("\n");
    }
    else {
        printf("1 SHA-256 Success\n");
    }

    sha256_final(&ctx2, sha256_result);

    if (memcmp(sha256_result, expected2, SHA256_DIGEST_LENGTH) != 0)
    {
        printf("2 SHA-256 Failure\n");

        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        {
            printf("%02x ", sha256_result[i]);
        }
        printf("\n");
    }
    else
    {
        printf("2 SHA-256 Success\n");
    }
}

static void aes_test(void)
{
    int err;
    cipher_context_t c_ctx;
    uint8_t data[AES_BLOCK_SIZE];
    memset(data, 0, AES_BLOCK_SIZE);

    err = aes_init(&c_ctx, TEST_0_KEY, AES_KEY_SIZE);
    if (err < 1) {
        printf("AES Init failed: %d\n", err);
    }

    start = xtimer_now_usec();
    if (aes_encrypt(&c_ctx, TEST_0_INP, data)) {
        stop = xtimer_now_usec();
        t_diff = stop - start;
        printf("AES Encrypt time: %ld us\n", t_diff);
        if (!memcmp(data, TEST_0_ENC, AES_BLOCK_SIZE)) {
            printf("AES encryption successful\n");
        }
        else
        {
            printf("AES encryption failed\n");
            for (int i = 0; i < 16; i++) {
                printf("%02x ", data[i]);
            }
            printf("\n");
        }
    }
    memset(data, 0, AES_BLOCK_SIZE);
    start = xtimer_now_usec();
    if (aes_decrypt(&c_ctx, TEST_0_ENC, data)) {
        stop = xtimer_now_usec();
        t_diff = stop - start;
        printf("AES Decrypt time: %ld us\n", t_diff);
        if (!memcmp(data, TEST_0_INP, AES_BLOCK_SIZE)) {
            printf("AES decryption successful\n");
        }
        else
        {
            printf("AES decryption failed\n");
            for (int i = 0; i < 16; i++) {
                printf("%02x ", data[i]);
            }
            printf("\n");
        }
    }
}

int main(void)
{
    puts("Hello World!");
    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);

#ifdef NRF52840_XXAA
    // Initialize crypto subsystem
    if (nrf_crypto_init() != NRF_SUCCESS) {
        printf("Error initializing nrf backend\n");
    }
    if (nrf_mem_init() != NRF_SUCCESS) {
        printf("Error initializing nrf memory\n");
    }
    nrf_sdk_sha256();
    nrf_sdk_aes();
#elif ARM_CRYPTOCELL
    cryptocell_setup();
    sha1_test();
    sha256_test();
    aes_test();
    cryptocell_terminate();
#else
    sha1_test();
    sha256_test();
    aes_test();
#endif
    return 0;
}
