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
#include "mmcau.h"
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

uint32_t t_start[2], t_stop[2];
uint32_t time[5];

void sha1_init(sha1_context *ctx)
{
    DEBUG("SHA1 init HW accelerated implementation\n");
    /* Initialize hash variables */
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xc3d2e1f0;
    ctx->byte_count = 0;
    ctx->buffer_offset = 0;
}

static void sha1_step(int count, int *i, int func, int constant, int *w)
{
    int index = *i;
    for (int j = 0; j < count; j++) {
            CAU->DIRECT[0] = MMCAU_2_CMDS((HASH+func), (ADRA+CA4));
            CAU->ADR_CAA = constant;
            CAU->LDR_CA[5] = w[index-16];
            CAU->XOR_CA[5] = w[index-14];
            CAU->XOR_CA[5] = w[index-8];
            CAU->XOR_CA[5] = w[index-3];
            CAU->ROTL_CA[5] = 1;
            w[index++] = CAU->STR_CA[5];
            CAU->DIRECT[0] = MMCAU_2_CMDS((ADRA+CA5), SHS);
        }
    *i = index;
}

#if ENABLE_DEBUG
static void sha1_hash_block(sha1_context *ctx)
{
    int j;
    int i = 0;
    int w[80];
    t_start[0] = xtimer_now_usec();
    for (j = 0; j < 5; j++) {
        CAU->LDR_CA[j] = ctx->state[j];
    }

    CAU->DIRECT[0] = MMCAU_1_CMD((MVRA+CA0));
    CAU->ROTL_CAA = 5;

    t_start[1] = xtimer_now_usec();
    for (j = 0; j < 16; j++) {
        w[i] = ctx->buffer[i];
        CAU->DIRECT[0] = MMCAU_2_CMDS((HASH+HFC), (ADRA+CA4));
        CAU->ADR_CAA = SHA1_K0;

        CAU->ADR_CAA = w[i++];
        CAU->DIRECT[0] = MMCAU_1_CMD(SHS);
    }

    /* perform hash algorithm on input */
    sha1_step(4, &i, HFC, SHA1_K0, w);
    t_stop[1] = xtimer_now_usec();
    time[1] = t_stop[1] - t_start[1];

    t_start[1] = xtimer_now_usec();
    sha1_step(20, &i, HFP, SHA1_K20, w);
    t_stop[1] = xtimer_now_usec();
    time[2] = t_stop[1] - t_start[1];

    t_start[1] = xtimer_now_usec();
    sha1_step(20, &i, HFM, SHA1_K40, w);
    t_stop[1] = xtimer_now_usec();
    time[3] = t_stop[1] - t_start[1];

    t_start[1] = xtimer_now_usec();
    sha1_step(20, &i, HFP, SHA1_K60, w);
    t_stop[1] = xtimer_now_usec();
    time[4] = t_stop[1] - t_start[1];

    for (j = 0; j < 5; j++) {
        CAU->ADR_CA[j] = ctx->state[j];
        ctx->state[j] = CAU->STR_CA[j];
    }

    t_stop[0] = xtimer_now_usec();
    time[0] = t_stop - t_start;
    DEBUG("Time:\nSha Step K0: %ld us\nSha Step K20: %ld us\nSha Step K40: %ld us\nSha Step K60: %ld us\nHash Block total: %ld us\n", time[1], time[2], time[3], time[4], time[0]);
}
#else
static void sha1_hash_block(sha1_context *ctx)
{
    int j;
    int i = 0;
    int w[80];
    for (j = 0; j < 5; j++) {
        CAU->LDR_CA[j] = ctx->state[j];
    }

    CAU->DIRECT[0] = MMCAU_1_CMD((MVRA+CA0));
    CAU->ROTL_CAA = 5;

    for (j = 0; j < 16; j++) {
        w[i] = ctx->buffer[i];
        CAU->DIRECT[0] = MMCAU_2_CMDS((HASH+HFC), (ADRA+CA4));
        CAU->ADR_CAA = SHA1_K0;

        CAU->ADR_CAA = w[i++];
        CAU->DIRECT[0] = MMCAU_1_CMD(SHS);
    }

    /* perform hash algorithm on input */
    sha1_step(4, &i, HFC, SHA1_K0, w);
    sha1_step(20, &i, HFP, SHA1_K20, w);
    sha1_step(20, &i, HFM, SHA1_K40, w);
    sha1_step(20, &i, HFP, SHA1_K60, w);

    for (j = 0; j < 5; j++) {
        CAU->ADR_CA[j] = ctx->state[j];
        ctx->state[j] = CAU->STR_CA[j];
    }
}
#endif /* ENABLE_DEBUG */

static void sha1_add_uncounted(sha1_context *s, uint8_t data)
{
    uint8_t *const b = (uint8_t *) s->buffer;

#ifdef __BIG_ENDIAN__
    b[s->buffer_offset] = data;
#else
    b[s->buffer_offset ^ 3] = data;
#endif
    s->buffer_offset++;
    if (s->buffer_offset == SHA1_BLOCK_LENGTH) {
        sha1_hash_block(s);
        s->buffer_offset = 0;
    }
}

static void sha1_update_byte(sha1_context *ctx, uint8_t data)
{
    ++ctx->byte_count;
    sha1_add_uncounted(ctx, data);
}

void sha1_update(sha1_context *ctx, const void *data, size_t len)
{
    const uint8_t *d = data;
    while (len--) {
        sha1_update_byte(ctx, *(d++));
    }
}

static void sha1_pad(sha1_context *s)
{
    /* Implement SHA-1 padding (fips180-2 §5.1.1) */
    /* Pad with 0x80 followed by 0x00 until the end of the block */
    sha1_add_uncounted(s, 0x80);
    while (s->buffer_offset != 56) {
        sha1_add_uncounted(s, 0x00);
    }

    /* Append length in the last 8 bytes */
    sha1_add_uncounted(s, 0);                   /* We're only using 32 bit lengths */
    sha1_add_uncounted(s, 0);                   /* But SHA-1 supports 64 bit lengths */
    sha1_add_uncounted(s, 0);                   /* So zero pad the top bits */
    sha1_add_uncounted(s, s->byte_count >> 29); /* Shifting to multiply by 8 */
    sha1_add_uncounted(s, s->byte_count >> 21); /* as SHA-1 supports bitstreams as well as */
    sha1_add_uncounted(s, s->byte_count >> 13); /* byte. */
    sha1_add_uncounted(s, s->byte_count >> 5);
    sha1_add_uncounted(s, s->byte_count << 3);
}

void sha1_final(sha1_context *ctx, void *digest)
{
    /* Pad to complete the last block */
    sha1_pad(ctx);

    /* Swap byte order back */
    for (int i = 0; i < 5; i++) {
        ctx->state[i] =
            (((ctx->state[i]) << 24) & 0xff000000)
            | (((ctx->state[i]) << 8) & 0x00ff0000)
            | (((ctx->state[i]) >> 8) & 0x0000ff00)
            | (((ctx->state[i]) >> 24) & 0x000000ff);
    }

    /* Copy the content of the hash (20 characters) */
    memcpy(digest, ctx->state, 20);
}

#ifdef FREESCALE_MMCAU
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

    static void mmcau_sha1(unsigned int *state, const void *data, void *digest, unsigned int len)
    {
        unsigned char *sha1_padded;
        sha1_padded = mmcau_sha_pad(data, &len, 1);
        int blocks = len/SHA1_BLOCK_LENGTH;
        cau_sha1_update(sha1_padded, blocks, state);
        /* Copy the content of the hash (20 characters) */

        /* Swap byte order back */
        for (int i = 0; i < 5; i++) {
            state[i] =
                (((state[i]) << 24) & 0xff000000)
                | (((state[i]) << 8) & 0x00ff0000)
                | (((state[i]) >> 8) & 0x0000ff00)
                | (((state[i]) >> 24) & 0x000000ff);
        }
        memcpy(digest, state, 20);
        free(sha1_padded);
    }
#endif
void sha1(void *digest, const void *data, size_t len)
{
#ifdef FREESCALE_MMCAU
    unsigned int sha1_state[SHA1_BLOCK_LENGTH/4];
    sha1_state[0] = 0x67452301;
    sha1_state[1] = 0xefcdab89;
    sha1_state[2] = 0x98badcfe;
    sha1_state[3] = 0x10325476;
    sha1_state[4] = 0xc3d2e1f0;
    mmcau_sha1(sha1_state, data, digest, (unsigned int) len);
#else
    sha1_context ctx;
    sha1_init(&ctx);
    sha1_update(&ctx, (unsigned char *) data, len);
    sha1_final(&ctx, digest);
#endif
}

#define HMAC_IPAD 0x36
#define HMAC_OPAD 0x5c

void sha1_init_hmac(sha1_context *ctx, const void *key, size_t key_length)
{
    uint8_t i;
    const uint8_t *k = key;

    memset(ctx->key_buffer, 0, SHA1_BLOCK_LENGTH);
    if (key_length > SHA1_BLOCK_LENGTH) {
        /* Hash long keys */
        sha1_init(ctx);
        while (key_length--) {
            sha1_update_byte(ctx, *k++);
        }
        sha1_final(ctx, ctx->key_buffer);
    }
    else {
        /* Block length keys are used as is */
        memcpy(ctx->key_buffer, key, key_length);
    }
    /* Start inner hash */
    sha1_init(ctx);
    for (i = 0; i < SHA1_BLOCK_LENGTH; i++) {
        sha1_update_byte(ctx, ctx->key_buffer[i] ^ HMAC_IPAD);
    }
}

void sha1_final_hmac(sha1_context *ctx, void *digest)
{
    uint8_t i;

    /* Complete inner hash */
    sha1_final(ctx, ctx->inner_hash);
    /* Calculate outer hash */
    sha1_init(ctx);
    for (i = 0; i < SHA1_BLOCK_LENGTH; i++) {
        sha1_update_byte(ctx, ctx->key_buffer[i] ^ HMAC_OPAD);
    }
    for (i = 0; i < SHA1_DIGEST_LENGTH; i++) {
        sha1_update_byte(ctx, ctx->inner_hash[i]);
    }

    sha1_final(ctx, digest);
}
