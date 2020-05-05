/*
 * Copyright (C) 2016 Oliver Hahm <oliver.hahm@inria.fr>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/* This code is public-domain - it is based on libcrypt
 * placed in the public domain by Wei Dai and other contributors.
 */

/**
 * @ingroup     sys_hashes_sha1

 * @{
 *
 * @file
 * @brief       Implementation of the SHA-1 hashing function
 *
 * @author      Lena Boeckmann <lena.boeckmann@haw-hamburg.de>
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "crypto/aes.h"
#include "crypto/ciphers.h"

#include "vendor/MKW21D5.h"
#include "mmcau.h"
#include "xtimer.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"


/**
 * Interface to the aes cipher
 */
static const cipher_interface_t aes_interface = {
    AES_BLOCK_SIZE,
    AES_KEY_SIZE,
    aes_init,
    aes_encrypt,
    aes_decrypt
};
const cipher_id_t CIPHER_AES_128 = &aes_interface;

/* for 128-bit blocks, Rijndael never uses more than 10 rcon values */
static const u32 rcon[] = {
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
    0x1B000000, 0x36000000,
};


int aes_init(cipher_context_t *context, const uint8_t *key, uint8_t keySize)
{
    DEBUG("AES init HW accelerated implementation\n");
    uint8_t i;
    /* This implementation only supports a single key size (defined in AES_KEY_SIZE) */
    if (keySize != AES_KEY_SIZE) {
        return CIPHER_ERR_INVALID_KEY_SIZE;
    }

    /* Make sure that context is large enough. If this is not the case,
       you should build with -DAES */
    if (CIPHER_MAX_CONTEXT_SIZE < AES_KEY_SIZE) {
        return CIPHER_ERR_BAD_CONTEXT_SIZE;
    }

    /* key must be at least CIPHERS_MAX_KEY_SIZE Bytes long */
    if (keySize < CIPHERS_MAX_KEY_SIZE) {
        /* fill up by concatenating key to as long as needed */
        for (i = 0; i < CIPHERS_MAX_KEY_SIZE; i++) {
            context->context[i] = key[(i % keySize)];
        }
    }
    else {
        for (i = 0; i < CIPHERS_MAX_KEY_SIZE; i++) {
            context->context[i] = key[i];
        }
    }

    return CIPHER_INIT_SUCCESS;
}

/**
 * Expand the cipher key into the encryption key schedule.
 */
static int aes_set_key(const unsigned char *userKey, const int bits,
                               AES_KEY *key)
{
    int i;
    int j;
    u32 *rk;
    rk = key->rd_key;

    if (!userKey || !key) {
        return -1;
    }

    if (bits != 128 && bits != 192 && bits != 256) {
        return -2;
    }

    if (bits == 256) {
        key->rounds = 14;
        for (i = 0; i < 8; i++) {
            rk[i] = GETU32(userKey + (i*4));
        }

        CAU->ADR_CAA = rk[i-1];

        for (j = 0; j < 6; j++) {
            CAU->ROTL_CAA = 8;
            CAU->DIRECT[0] = MMCAU_1_CMD((AESS+CAA));
            CAU->XOR_CAA = rcon[j];

            for (int k = 0; k < 8; k++) {
                CAU->XOR_CAA = rk[i-8];
                rk[i++] = CAU->STR_CAA;
            }
        }

        CAU->ROTL_CAA = 8;
        CAU->DIRECT[0] = MMCAU_1_CMD((AESS+CAA));
        CAU->XOR_CAA = rcon[j];

        for (int k = 0; k < 4; k++) {
            CAU->XOR_CAA = rk[i-8];
            rk[i++] = CAU->STR_CAA;
        }
    }

    if (bits == 192) {
        key->rounds = 12;
        for (i = 0; i < 6; i++) {
            rk[i] = GETU32(userKey + (i*4));;
        }

        CAU->LDR_CAA = rk[i-1];

        for (j = 0; j < 7; j++) {
            CAU->ROTL_CAA = 8;
            CAU->DIRECT[0] = MMCAU_1_CMD((AESS+CAA));
            CAU->XOR_CAA = rcon[j];

            for (int k = 0; k < 6; k++) {
                CAU->XOR_CAA = rk[i-6];
                rk[i++] = CAU->STR_CAA;
            }
        }

        CAU->ROTL_CAA = 8;
        CAU->DIRECT[0] = MMCAU_1_CMD((AESS+CAA));
        CAU->XOR_CAA = rcon[j];

        for (int k = 0; k < 4; k++) {
            CAU->XOR_CAA = rk[i-6];
            rk[i++] = CAU->STR_CAA;
        }
    }

    if (bits == 128) {
        key->rounds = 10;
        for (i = 0; i < 4; i++) {
            rk[i] = GETU32(userKey + (i*4));
        }
        CAU->LDR_CAA = rk[i-1];

        for (j = 0; j < 10; j++) {
            CAU->ROTL_CAA = 8;
            CAU->DIRECT[0] = MMCAU_1_CMD((AESS+CAA));
            CAU->XOR_CAA = rcon[j];

            for (int k = 0; k < 4; k++) {
                CAU->XOR_CAA = rk[i-4];
                rk[i++] = CAU->STR_CAA;
            }
        }
    }

    return 0;
}

/*
 * Encrypt a single block
 * in and out can overlap
 */
int aes_encrypt(const cipher_context_t *context, const uint8_t *plainBlock,
                uint8_t *cipherBlock)
{
    /* setup AES_KEY */
    int res;
    int i,j;
    int rounds;
    const u32 *rk;
    AES_KEY aeskey;
    const AES_KEY *key = &aeskey;

    res = aes_set_key((unsigned char *)context->context,
                              AES_KEY_SIZE * 8, &aeskey);

    if (res < 0) {
        return res;
    }

    rk = key->rd_key;
    rounds = key->rounds;
    /* Load plaintext into CA-Registers and XOR with first 4 keys*/
    for (int k = 0; k < 4; k++) {
        CAU->LDR_CA[k] = GETU32(plainBlock +(k*4));;
        CAU->XOR_CA[k] = rk[k];
    }

    /* Perform encryption */
    for (i = 0, j = 4; i < (rounds-1); i++, j += 4) {
        CAU->DIRECT[0] = MMCAU_3_CMDS((AESS+CA0), (AESS+CA1), (AESS+CA2));
        CAU->DIRECT[0] = MMCAU_2_CMDS((AESS+CA3), AESR);
        CAU->AESC_CA[0] = rk[j];
        CAU->AESC_CA[1] = rk[j+1];
        CAU->AESC_CA[2] = rk[j+2];
        CAU->AESC_CA[3] = rk[j+3];
    }

    CAU->DIRECT[0] = MMCAU_3_CMDS((AESS+CA0), (AESS+CA1), (AESS+CA2));
    CAU->DIRECT[0] = MMCAU_2_CMDS((AESS+CA3), AESR);
    CAU->XOR_CA[0] = rk[j];
    CAU->XOR_CA[1] = rk[j+1];
    CAU->XOR_CA[2] = rk[j+2];
    CAU->XOR_CA[3] = rk[j+3];

    /* Store cipher into cipherBlock */
    for (int k = 0; k < 4; k++) {
        PUTU32(cipherBlock + (4*k), CAU->STR_CA[k]);
    }
    return 1;
}

/*
 * Decrypt a single block
 * in and out can overlap
 */
int aes_decrypt(const cipher_context_t *context, const uint8_t *cipherBlock,
                uint8_t *plainBlock)
{
    /* setup AES_KEY */
    int res;
    int i;
    int rounds;
    const u32 *rk;
    AES_KEY aeskey;
    const AES_KEY *key = &aeskey;

#if ENABLE_DEBUG
    /* Timer variables */
    uint32_t start, stop, t_diff;
    start = xtimer_now_usec();
    res = aes_set_key((unsigned char *)context->context,
                              AES_KEY_SIZE * 8, &aeskey);
    stop = xtimer_now_usec();
    t_diff = stop-start;
    printf("HW Decrypt Key: %ld us \n", t_diff);
#else
    res = aes_set_key((unsigned char *)context->context,
                              AES_KEY_SIZE * 8, &aeskey);
#endif

    if (res < 0) {
        return res;
    }

    rk = key->rd_key;
    rounds = key->rounds;
    i = 4 * (rounds + 1);

    for (int k = 0; k < 4; k++) {
        CAU->LDR_CA[k] = GETU32(cipherBlock + (4*k));
    }
    for (int k = 3; k >= 0; k--) {
        CAU->XOR_CA[k] = rk[--i];
    }

    /* Perform decryption */
    while (i > 4) {
        CAU->DIRECT[0] = MMCAU_3_CMDS(AESIR, (AESIS+CA3), (AESIS+CA2));
        CAU->DIRECT[0] = MMCAU_2_CMDS((AESIS+CA1), (AESIS+CA0));
        CAU->AESIC_CA[3] = rk[--i];
        CAU->AESIC_CA[2] = rk[--i];
        CAU->AESIC_CA[1] = rk[--i];
        CAU->AESIC_CA[0] = rk[--i];
    }

    CAU->DIRECT[0] = MMCAU_3_CMDS(AESIR, (AESIS+CA3), (AESIS+CA2));
    CAU->DIRECT[0] = MMCAU_2_CMDS((AESIS+CA1), (AESIS+CA0));
    CAU->XOR_CA[3] = rk[--i];
    CAU->XOR_CA[2] = rk[--i];
    CAU->XOR_CA[1] = rk[--i];
    CAU->XOR_CA[0] = rk[--i];

    /* Store plaintext */
    for (int k = 0; k < 4; k++) {
        PUTU32(plainBlock + (4*k), CAU->STR_CA[k]);
    }
    return 1;
}
