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
#include "cc310.h"
#include "board.h"
#include "od.h"

#include "kernel_defines.h"

#include "nrf_cc3xx_platform.h"
#include "nrf_cc3xx_platform_entropy.h"
#include "nrf_cc3xx_platform_kmu.h"
#include "platform_alt.h"
// #include "nrf-config-cc310.h"
// #include "platform_alt.h"
#include "cc3xx_kmu.h"

#include "aes_driver.h"

#include "platform.h"

#include "mbedtls/aes.h"

#define ENTROPY_LENGTH  16
uint8_t entropy_buffer[ENTROPY_LENGTH] = { 0 };

#define KEY_SLOT 2

nrf_cc3xx_platform_key_buff_t key_buff = {
    .buff_128 = {
        '1', '2', '3', '4', '5', '6', '7', '8',
        '9', '0', '1', '2', '3', '4', '5', '6'
    }
};

static const unsigned char new_key[] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
    'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'
};

const unsigned char plain_text[32] = "12345678912345612345678912345";

int main(void)
{
    LED0_ON;
    puts("Hello World!");

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);
    int res;

    res = cc310_init();
    if (!res) {
        puts("Success initializing CC310");
        LED1_ON;
    }
    else {
        puts("ERROR initializing CC310");
        LED0_OFF;
        return 1;
    }

    if (nrf_cc3xx_platform_rng_is_initialized()) {
        puts("RNG enabled");
        LED2_ON;
    }
    else {
        puts("RNG not enabled");
        LED0_OFF;
        LED1_OFF;
    }

    size_t entropy_length = 0;
    res = nrf_cc3xx_platform_entropy_get(entropy_buffer, ENTROPY_LENGTH, &entropy_length);
    if (!res) {
        puts("Success getting entropy");
        printf("Got %d bytes\n", entropy_length);
        od_hex_dump(entropy_buffer, ENTROPY_LENGTH, 0);
    }
    else {
        puts("ERROR getting entropy");
    }

    puts("Testing key management unit (KMU)");

    /* we test with AES-128 */
    res = nrf_cc3xx_platform_kmu_write_key(KEY_SLOT, NRF_CC3XX_PLATFORM_KEY_TYPE_AES_128_BIT, key_buff);
    if (NRF_CC3XX_PLATFORM_SUCCESS == res) {
        puts("Success writing key");
        LED3_ON;
    }
    else {
        puts("ERROR writing key");
    }

    // we don't need it, internally does the same as platform_init in cc310.c
    //
    static mbedtls_platform_context platform_context = {0};
    res = mbedtls_platform_setup(&platform_context);
    if (res != 0) {
            puts("ERROR initializing mbedtls");
            return res;
    }
    else {
        puts("Mbedtls initialized");
    }

    mbedtls_aes_context context = {0};
    mbedtls_aes_init(&context);

    res = mbedtls_aes_setkey_enc_shadow_key(&context, KEY_SLOT, 128);
    if (!res) {
        puts("Set key slot 2 as AES key");
    }
    else {
        puts("ERROR setting key slot 2 as AES key");
    }

    (void) new_key;
    // res = mbedtls_aes_setkey_enc(&context, new_key, 128);
    // if (!res) {
    //     puts("Set new key as AES key");
    // }
    // else {
    //     puts("ERROR setting new key as AES key");
    // }

    AesContext_t *ctx = (AesContext_t *)(&context);
    printf("Context key type: %d\n", ctx->cryptoKey);

    unsigned char output[2 * ARRAY_SIZE(plain_text)] = {0};

    // uint8_t iv[16] = {0};
    res = mbedtls_aes_crypt_cbc(&context, MBEDTLS_AES_ENCRYPT, ARRAY_SIZE(plain_text), entropy_buffer, plain_text, output);

    // res = mbedtls_internal_aes_encrypt(&context, plain_text, output);
    if (!res) {
        puts("Successfully encrypted text");
    }
    else {
        printf("ERROR encrypting text 0x%04x\n", -res);
    }
    od_hex_dump(output, ARRAY_SIZE(output), 0);

    return 0;
}
