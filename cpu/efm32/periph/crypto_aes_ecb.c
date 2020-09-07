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

#define ENABLE_DEBUG    (0)
#include "debug.h"


/*
 * Encrypt a single block
 * in and out can overlap
 */
int aes_encrypt_ecb(cipher_context_t *context, const uint8_t *input,
                       size_t length, uint8_t *output)
{
    CRYPTO_TypeDef* dev = crypto_acquire();

    CRYPTO_AES_ECB128(dev, output, input, length, context->context, true);

    crypto_release(dev);
    return 1;
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

    gpio_set(GPIO_PIN(2, 7));
    CRYPTO_AES_DecryptKey128(dev, decrypt_key, context->context);
    gpio_clear(GPIO_PIN(2, 7));
    CRYPTO_AES_ECB128(dev, output, input, length, decrypt_key, false);

    crypto_release(dev);
    return 1;
}
