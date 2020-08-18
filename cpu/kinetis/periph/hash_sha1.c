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

#include <stdint.h>
#include <string.h>

#include "vendor/MKW21D5.h"
#include "hashes/sha1.h"
#include "cau_api.h"
#include "xtimer.h"

#ifdef FREESCALE_MMCAU
#include <stdlib.h>
#include "cau_api.h"
#endif

#define ENABLE_DEBUG    (0)
#include "debug.h"

#define SHA1_K0  0x5a827999
#define SHA1_K20 0x6ed9eba1
#define SHA1_K40 0x8f1bbcdc
#define SHA1_K60 0xca62c1d6

void sha1_init(sha1_context *ctx)
{
    DEBUG("SHA1 init HW accelerated implementation\n");
    /* Initialize hash variables */
    cau_sha1_initialize_output(ctx->sha1_state);
}
/* ATTENTION – The following padding function has been copy-pasted from a Freescale coding example, which can be downloaded here: https://www.nxp.com/docs/en/application-note-software/AN4307SW.zip

The padding function implemented in RIOT can't be separated from the hashing function. To use the mmCAU hashing functions a separate padding function is needed. */
static void *mmcau_sha_pad(const void *data, unsigned int *len, unsigned char endianess)
{
    unsigned char *padding_array;
    signed char padding_length;
    unsigned int temp_length;
    unsigned int bits_length;
    unsigned int final_length;

    temp_length = *len % SHA1_BLOCK_LENGTH;

    /*get padding length: padding special case when 448 mod 512*/
    /*working with bytes rather than bits*/
    if( !((padding_length = 56-(temp_length%SHA1_BLOCK_LENGTH)) > 0) )
        padding_length = SHA1_BLOCK_LENGTH - (temp_length%56);

    padding_length +=  temp_length/SHA1_BLOCK_LENGTH;
    temp_length = *len;

    /*reserve necessary memory*/
    final_length = temp_length + padding_length + 8/*bits length*/;
    if( (padding_array = (unsigned char *)malloc(final_length)) == NULL )
        return (unsigned char *)NULL;/*not enough mem*/

    /*copy original data to new padding array*/
    memcpy((void*)padding_array,data,temp_length);

    /*add padding*/
    padding_array[temp_length++] = 0x80;/*first bit enabled*/
    while((--padding_length != 0))
        padding_array[temp_length++] = 0;/*clear the rest*/

    /*add length depending on endianess: get number of bits*/
    bits_length = *len << 3;
    *len = final_length;

    if( endianess == 0 )
    {
        padding_array[temp_length++] = bits_length     & 0xff;
        padding_array[temp_length++] = bits_length>>8  & 0xff;
        padding_array[temp_length++] = bits_length>>16 & 0xff;
        padding_array[temp_length++] = bits_length>>24 & 0xff;
        padding_array[temp_length++] = 0;
        padding_array[temp_length++] = 0;
        padding_array[temp_length++] = 0;
        padding_array[temp_length  ] = 0;
    }
    else/*CRYPTO_BIG_ENDIAN*/
    {
        padding_array[temp_length++] = 0;
        padding_array[temp_length++] = 0;
        padding_array[temp_length++] = 0;
        padding_array[temp_length++] = 0;
        padding_array[temp_length++] = bits_length>>24 & 0xff;
        padding_array[temp_length++] = bits_length>>16 & 0xff;
        padding_array[temp_length++] = bits_length>>8  & 0xff;
        padding_array[temp_length  ] = bits_length     & 0xff;
    }

    return padding_array;
}

static void mmcau_sha1(unsigned int *state, const void *data, unsigned int len)
{
    unsigned char *sha1_padded;
    sha1_padded = mmcau_sha_pad(data, &len, 1);
    int blocks = len/SHA1_BLOCK_LENGTH;
    cau_sha1_update(sha1_padded, blocks, state);
    free(sha1_padded);
}

void sha1_update(sha1_context *ctx, const void *data, size_t len)
{
    mmcau_sha1(ctx->sha1_state, data, len);
}

void sha1_final(sha1_context *ctx, void *digest)
{
    /* Swap byte order back */
    for (int i = 0; i < 5; i++) {
        ctx->sha1_state[i] =
            (((ctx->sha1_state[i]) << 24) & 0xff000000)
            | (((ctx->sha1_state[i]) << 8) & 0x00ff0000)
            | (((ctx->sha1_state[i]) >> 8) & 0x0000ff00)
            | (((ctx->sha1_state[i]) >> 24) & 0x000000ff);
    }
    memcpy(digest, &ctx->sha1_state, SHA1_DIGEST_LENGTH);
}

void sha1(void *digest, const void *data, size_t len)
{
    sha1_context ctx;
    sha1_init(&ctx);
    sha1_update(&ctx, (unsigned char *) data, len);
    sha1_final(&ctx, digest);
}

#define HMAC_IPAD 0x36
#define HMAC_OPAD 0x5c

void sha1_init_hmac(sha1_context *ctx, const void *key, size_t key_length)
{
    (void) ctx;
    (void) key;
    (void) (key_length);
    // uint8_t i;
    // const uint8_t *k = key;

    // memset(ctx->key_buffer, 0, SHA1_BLOCK_LENGTH);
    // if (key_length > SHA1_BLOCK_LENGTH) {
    //     /* Hash long keys */
    //     sha1_init(ctx);
    //     while (key_length--) {
    //         sha1_update_byte(ctx, *k++);
    //     }
    //     sha1_final(ctx, ctx->key_buffer);
    // }
    // else {
    //     /* Block length keys are used as is */
    //     memcpy(ctx->key_buffer, key, key_length);
    // }
    // /* Start inner hash */
    // sha1_init(ctx);
    // for (i = 0; i < SHA1_BLOCK_LENGTH; i++) {
    //     sha1_update_byte(ctx, ctx->key_buffer[i] ^ HMAC_IPAD);
    // }
}

void sha1_final_hmac(sha1_context *ctx, void *digest)
{
    (void) ctx;
    (void) digest;
    // uint8_t i;

    // /* Complete inner hash */
    // sha1_final(ctx, ctx->inner_hash);
    // /* Calculate outer hash */
    // sha1_init(ctx);
    // for (i = 0; i < SHA1_BLOCK_LENGTH; i++) {
    //     sha1_update_byte(ctx, ctx->key_buffer[i] ^ HMAC_OPAD);
    // }
    // for (i = 0; i < SHA1_DIGEST_LENGTH; i++) {
    //     sha1_update_byte(ctx, ctx->inner_hash[i]);
    // }

    // sha1_final(ctx, digest);
}
