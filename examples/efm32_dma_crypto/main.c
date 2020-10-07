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
#include "kernel_defines.h"
#include "crypto_util.h"
#include "mutex.h"
#include "em_ldma.h"
#include "em_device.h"

/* This header will define, according to the Kconfig configuration:
 *   - key_0 (key, array of uint_8, length: 16 bytes)
 *   - plainText (text to encrypt, array of uint8_t, length AES_NUMBYTES)
 *   - cipherText (ciphered text to compare, array of uint8_t, length AES_NUMBYTES)
 *   - AES_NUMBYTES (number of bytes to encrypt, macro)
 */
#include "test_vectors.h"

static uint8_t cipherData[AES_NUMBYTES];

int is_result_expected(uint8_t *result, uint8_t *expected, size_t bytes)
{
    for (unsigned i = 0; i < bytes; i++) {
        if (result[i] != expected[i]) {
            return 0;
        }
    }
    return 1;
}

int main(void)
{
    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);
    puts("=== Test AES on CRYPTO ===");
    printf("-> Number of bytes: %ld\n", AES_NUMBYTES);

    crypto_device_t *crypto = crypto_acquire_dev();

    /* set the key in the CRYPTO */
    /* when the key is not word-aligned we need to copy it to a buffer */
    /* it could be checked but now not only for simplicity */
    uint32_t key[AES_SIZE_WORDS];
    memcpy(key, key_0, AES_SIZE_BYTES);

    for (unsigned i = 0; i < AES_SIZE_WORDS; i++) {
        crypto->dev->KEYBUF = key[i];
    }

    if (IS_ACTIVE(CONFIG_AES_WITH_DMA)) {
        puts("== Testing AES with DMA ==");
        crypto_aes_128_encrypt(crypto, plainText, cipherData, AES_NUMBYTES);
    }
    else {
        puts("== Testing AES without DMA ==");


        /* load the plain text in the CRYPTO */
        for (int i = 0; i < 16; i++) {
            crypto->dev->DATA0BYTE = plainText[i];
        }

        crypto->dev->CTRL = CRYPTO_CTRL_AES_AES128; /* use AES-128 mode */

        /* trigger AES encryption */
        crypto->dev->CMD = CRYPTO_CMD_INSTR_AESENC;

        /* Wait for completion */
        while ((crypto->dev->IF & CRYPTO_IF_INSTRDONE) == 0UL) { }
        crypto->dev->IFC = CRYPTO_IF_INSTRDONE;

        // TODO: Here goes a loop!
        for (unsigned i = 0; i < AES_NUMBYTES; i++) {
            cipherData[i] = crypto->dev->DATA0BYTE;
        }
    }

    puts("-> Done");

    if (is_result_expected(cipherData, cipherText, AES_NUMBYTES)) {
            puts("=> CORRECT");
    }
    else {
        puts("=> INCORRECT!");
    }

    for (unsigned i = 0; i < AES_NUMBYTES; i++) {
        printf("[0x%x], ", cipherData[i]);
    }

    puts("");

}
