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

static inline void CRYPTO_DataReadUnaligned(volatile uint32_t * reg,
                                              uint8_t * val)
{
  /* Check data is 32bit aligned, if not, read into temporary buffer and
     then move to user buffer. */
  if ((uintptr_t)val & 0x3) {
    uint32_t temp[4];
    CRYPTO_DataRead(reg, temp);
    memcpy(val, temp, 16);
  } else {
    CRYPTO_DataRead(reg, (uint32_t *)val);
  }
}

/***************************************************************************//**
 * @brief
 *   Write 128 bits of data to a DATAX register in the CRYPTO module.
 *
 * @param[in]  dataReg    The 128 bit DATA register.
 * @param[in]  val        Pointer to value to write to the DATA register.
 ******************************************************************************/
static inline void CRYPTO_DataWriteUnaligned(volatile uint32_t * reg,
                                               const uint8_t * val)
{
  /* Check data is 32bit aligned, if not move to temporary buffer before
     writing.*/
  if ((uintptr_t)val & 0x3) {
    uint32_t temp[4];
    memcpy(temp, val, 16);
    CRYPTO_DataWrite(reg, temp);
  } else {
    CRYPTO_DataWrite(reg, (const uint32_t *)val);
  }
}

static inline void CRYPTO_AES_Process(CRYPTO_TypeDef *        crypto,
                                      unsigned int            len,
                                      CRYPTO_DataReg_TypeDef  inReg,
                                      const uint8_t  *        in,
                                      CRYPTO_DataReg_TypeDef  outReg,
                                      uint8_t *               out)
{
    len /= CRYPTO_AES_BLOCKSIZE;
    crypto->SEQCTRL = 16UL << _CRYPTO_SEQCTRL_LENGTHA_SHIFT;

    if (((uintptr_t)in & 0x3) || ((uintptr_t)out & 0x3)) {
        while (len > 0UL) {
            len--;
            /* Load data and trigger encryption. */
            CRYPTO_DataWriteUnaligned(inReg, in);

            crypto_wait_for_sequence(crypto);

            /* Save encrypted/decrypted data. */
            CRYPTO_DataReadUnaligned(outReg, out);

            out += 16;
            in += 16;
        }
    }
    else {
        /* Optimized version, 15% faster for -O3. */
        if (len > 0UL) {
            /* Load first data */
            CRYPTO_DataWrite(inReg, (uint32_t *)in);

            /* Do loop administration */
            in += 16;
            len--;

            while (len > 0UL) {
                crypto_wait_for_sequence(crypto);
                /* Save encrypted/decrypted data. */
                CRYPTO_DataRead(outReg, (uint32_t *)out);

                /* Load next data and retrigger encryption asap. */
                CRYPTO_DataWrite(inReg, (uint32_t *)in);

                /* Do loop administration */
                out += 16;
                in += 16;
                len--;
            }

            crypto_wait_for_sequence(crypto);
            /* Save last encrypted/decrypted data. */
            CRYPTO_DataRead(outReg, (uint32_t *)out);
        }
    }
}

/*
 * Encrypt a single block
 * in and out can overlap
 */
int aes_encrypt_ecb(cipher_context_t *context, const uint8_t *input,
                       size_t length, uint8_t *output)
{
    CRYPTO_TypeDef* dev = crypto_acquire();

    if (IS_ACTIVE(CONFIG_EFM32_AES_ECB_NONBLOCKING)) {
        /* set AES-128 mode */
        dev->CTRL = CRYPTO_CTRL_AES_AES128;

        dev->WAC = 0;

        /* set the key value */
        const uint8_t *key = context->context;

        /* Check if key val buffer is 32bit aligned, if not move to temporary
           aligned buffer before writing.*/
        if ((uintptr_t)key & 0x3) {
            CRYPTO_KeyBuf_TypeDef temp;
            /* as we are using AES-128, 16 bytes are copied */
            memcpy(temp, key, 16);

            CRYPTO_KeyBufWrite(dev, temp, 8);
        } else {
            CRYPTO_KeyBufWrite(dev, (uint32_t*)key, 8);
        }

        CRYPTO_SEQ_LOAD_2(dev, CRYPTO_CMD_INSTR_AESENC,
                          CRYPTO_CMD_INSTR_DATA0TODATA1);


        CRYPTO_AES_Process(dev, length, &dev->DATA0, input,
                            &dev->DATA1, output);

    }
    else {
        CRYPTO_AES_ECB128(dev, output, input, length, context->context, true);
    }

    crypto_release(dev);
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
