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
#include "crypto_util.h"
#include "periph/gpio.h"

#include "em_device.h"
#include "em_crypto.h"
#include "em_bus.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#if TEST_AES_KEY
extern gpio_t gpio_aes_key;
// // fixme
// #include "crypto_runtime.h"
#endif

#define CRYPTO_AES_BLOCKSIZE (16UL)
/*
 * Encrypt a single block
 * in and out can overlap
 */
int aes_encrypt_ecb(cipher_context_t *context, const uint8_t *input,
                       size_t length, uint8_t *output)
{
    crypto_device_t *crypto = crypto_acquire_dev();

    gpio_set(crypto->pin);

    if (IS_ACTIVE(CONFIG_EFM32_AES_ECB_NONBLOCKING)) {
        /* set the key value */
        uint32_t key[AES_SIZE_WORDS];
        /* as we are using AES-128, 16 bytes are copied */
        memcpy(key, context->context, 16);

        for (unsigned i = 0; i < AES_SIZE_WORDS; i++) {
            crypto->dev->KEYBUF = key[i];
        }
        crypto_aes_128_encrypt(crypto, input, output, length);
    }
    else {
        CRYPTO_AES_ECB128(crypto->dev, output, input, length, context->context, true);
    }

    gpio_clear(crypto->pin);

    crypto_release_dev(crypto);
    return length;
}

/*
 * Decrypt a single block
 * in and out can overlap
 */
int aes_decrypt_ecb(cipher_context_t *context, const uint8_t *input,
                       size_t length, uint8_t *output)
{
    uint8_t decrypt_key[AES_KEY_SIZE];

    CRYPTO_TypeDef* dev = crypto_acquire();

#if TEST_AES_KEY
    gpio_set(gpio_aes_key);
#endif
    CRYPTO_AES_DecryptKey128(dev, decrypt_key, context->context);
#if TEST_AES_KEY
    gpio_clear(gpio_aes_key);
#endif
    CRYPTO_AES_ECB128(dev, output, input, length, decrypt_key, false);

    crypto_release(dev);
    return length;
}
