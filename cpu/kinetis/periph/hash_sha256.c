/*-
 * Copyright 2005 Colin Percival
 * Copyright 2013 Christian Mehlis & René Kijewski
 * Copyright 2016 Martin Landsmann <martin.landsmann@haw-hamburg.de>
 * Copyright 2016 OTA keys S.A.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/lib/libmd/sha256c.c,v 1.2 2006/01/17 15:35:56 phk Exp $
 */

/**
 * @ingroup     sys_hashes
 * @{
 *
 * @file
 * @brief       SHA256 hash function implementation
 *
 * @author      Lena Boeckmann
 *
 * @}
 */

#include <string.h>
#include <assert.h>

#include "vendor/MKW21D5.h"
#include "hashes/sha256.h"
#include "mmcau.h"

#ifdef FREESCALE_MMCAU
#include <stdlib.h>
#include "cau_api.h"
#endif

#define ENABLE_DEBUG    (0)
#include "debug.h"

#ifdef __BIG_ENDIAN__
/* Copy a vector of big-endian uint32_t into a vector of bytes */
#define be32enc_vect memcpy

/* Copy a vector of bytes into a vector of big-endian uint32_t */
#define be32dec_vect memcpy

#else /* !__BIG_ENDIAN__ */

/*
 * Encode a length len/4 vector of (uint32_t) into a length len vector of
 * (unsigned char) in big-endian form.  Assumes len is a multiple of 4.
 */
static void be32enc_vect(void *dst_, const void *src_, size_t len)
{
    if ((uintptr_t)dst_ % sizeof(uint32_t) == 0 &&
        (uintptr_t)src_ % sizeof(uint32_t) == 0) {
        uint32_t *dst = dst_;
        const uint32_t *src = src_;
        for (size_t i = 0; i < len / 4; i++) {
            dst[i] = __builtin_bswap32(src[i]);
        }
    }
    else {
        uint8_t *dst = dst_;
        const uint8_t *src = src_;
        for (size_t i = 0; i < len; i += 4) {
            dst[i] = src[i + 3];
            dst[i + 1] = src[i + 2];
            dst[i + 2] = src[i + 1];
            dst[i + 3] = src[i];
        }
    }
}

/*
 * Decode a big-endian length len vector of (unsigned char) into a length
 * len/4 vector of (uint32_t).  Assumes len is a multiple of 4.
 */
#define be32dec_vect be32enc_vect

#endif /* __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ */

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

/*
 * SHA256 block compression function.  The 256-bit state is transformed via
 * the 512-bit input block to produce a new state.
 */
static void sha256_transform(uint32_t *state, const unsigned char block[64])
{
    int j;
    int i = 0;
    uint32_t W[64];
    be32dec_vect(W, block, 64);

    for (int j = 0; j < 8; j++) {
        CAU->LDR_CA[j] = state[j];
    }

    for (j = 0; j < 16; j++) {
        CAU->LDR_CAA = W[i];
        CAU->DIRECT[0] = MMCAU_3_CMDS((ADRA+CA7), (HASH+HF2T), (HASH+HF2C));
        CAU->ADR_CAA = K[i++];
        CAU->DIRECT[0] = MMCAU_3_CMDS((MVAR+CA8), (HASH+HF2S), (HASH+HF2M));
        CAU->DIRECT[0] = MMCAU_1_CMD(SHS2);
    }

    for (j = 0; j < 48; j++) {
        CAU->LDR_CAA = W[i-16];
        CAU->LDR_CA[8] = W[i-15];
        CAU->DIRECT[0] = MMCAU_1_CMD((HASH+HF2U));
        CAU->ADR_CAA = W[i-7];
        CAU->LDR_CA[8] = W[i-2];
        CAU->DIRECT[0] = MMCAU_1_CMD((HASH+HF2V));
        W[i] = CAU->STR_CAA;
        CAU->DIRECT[0] = MMCAU_3_CMDS((ADRA+CA7), (HASH+HF2T), (HASH+HF2C));
        CAU->ADR_CAA = K[i++];
        CAU->DIRECT[0] = MMCAU_3_CMDS((MVAR+CA8), (HASH+HF2S), (HASH+HF2M));
        CAU->DIRECT[0] = MMCAU_1_CMD(SHS2);
    }

    for (j = 0; j < 8; j++) {
        CAU->ADR_CA[j] = state[j];
        state[j] = CAU->STR_CA[j];
    }
}

static unsigned char PAD[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/* Add padding and terminating bit-count. */
static void sha256_pad(sha256_context_t *ctx)
{
    /*
     * Convert length to a vector of bytes -- we do this now rather
     * than later because the length will change after we pad.
     */
    unsigned char len[8];

    be32enc_vect(len, ctx->count, 8);

    /* Add 1--64 bytes so that the resulting length is 56 mod 64 */
    uint32_t r = (ctx->count[1] >> 3) & 0x3f;
    uint32_t plen = (r < 56) ? (56 - r) : (120 - r);
    sha256_update(ctx, PAD, (size_t) plen);

    /* Add the terminating bit-count */
    sha256_update(ctx, len, 8);
}

/* SHA-256 initialization.  Begins a SHA-256 operation. */
void sha256_init(sha256_context_t *ctx)
{
    DEBUG("SHA256 init HW accelerated implementation\n");
    /* Zero bits processed so far */
    ctx->count[0] = ctx->count[1] = 0;

    /* Magic initialization constants */
    ctx->state[0] = 0x6A09E667;
    ctx->state[1] = 0xBB67AE85;
    ctx->state[2] = 0x3C6EF372;
    ctx->state[3] = 0xA54FF53A;
    ctx->state[4] = 0x510E527F;
    ctx->state[5] = 0x9B05688C;
    ctx->state[6] = 0x1F83D9AB;
    ctx->state[7] = 0x5BE0CD19;
}

/* Add bytes into the hash */
void sha256_update(sha256_context_t *ctx, const void *data, size_t len)
{
    /* Number of bytes left in the buffer from previous updates */
    uint32_t r = (ctx->count[1] >> 3) & 0x3f;

    /* Convert the length into a number of bits */
    uint32_t bitlen1 = ((uint32_t) len) << 3;
    uint32_t bitlen0 = ((uint32_t) len) >> 29;

    /* Update number of bits */
    if ((ctx->count[1] += bitlen1) < bitlen1) {
        ctx->count[0]++;
    }

    ctx->count[0] += bitlen0;

    /* Handle the case where we don't need to perform any transforms */
    if (len < 64 - r) {
        if (len > 0) {
            memcpy(&ctx->buf[r], data, len);
        }
        return;
    }

    /* Finish the current block */
    const unsigned char *src = data;

    memcpy(&ctx->buf[r], src, 64 - r);
    sha256_transform(ctx->state, ctx->buf);
    src += 64 - r;
    len -= 64 - r;

    /* Perform complete blocks */
    while (len >= 64) {
        sha256_transform(ctx->state, src);
        src += 64;
        len -= 64;
    }

    /* Copy left over data into buffer */
    memcpy(ctx->buf, src, len);
}

/*
 * SHA-256 finalization.  Pads the input data, exports the hash value,
 * and clears the context state.
 */
void sha256_final(sha256_context_t *ctx, void *dst)
{
    /* Add padding */
    sha256_pad(ctx);

    /* Write the hash */
    be32enc_vect(dst, ctx->state, 32);

    /* Clear the context state */
    memset((void *) ctx, 0, sizeof(*ctx));
}

#ifdef FREESCALE_MMCAU
    static void *mmcau_sha_pad(const void *data, unsigned int *len, unsigned char endianess)
    {
        unsigned char *padding_array;
        signed char padding_length;
        unsigned int temp_length;
        unsigned int bits_length;
        unsigned int final_length;

        temp_length = *len % SHA256_INTERNAL_BLOCK_SIZE;

        /*get padding length: padding special case when 448 mod 512*/
        /*working with bytes rather than bits*/
        if( !((padding_length = 56-(temp_length%SHA256_INTERNAL_BLOCK_SIZE)) > 0) )
            padding_length = SHA256_INTERNAL_BLOCK_SIZE - (temp_length%56);

        padding_length +=  temp_length/SHA256_INTERNAL_BLOCK_SIZE;
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

    static void mmcau_sha256(unsigned int *state, const void *data, void *digest, unsigned int len)
    {
        unsigned char *sha1_padded;
        sha1_padded = mmcau_sha_pad(data, &len, 1);
        int blocks = len/SHA256_INTERNAL_BLOCK_SIZE;
        cau_sha256_update(sha1_padded, blocks, state);
        /* Write the hash */
        be32enc_vect(digest, state, 32);
        free(sha1_padded);
    }
#endif

void *sha256(const void *data, size_t len, void *digest)
{
#ifdef FREESCALE_MMCAU
    unsigned int sha_state[8];
    sha_state[0] = 0x6A09E667;
    sha_state[1] = 0xBB67AE85;
    sha_state[2] = 0x3C6EF372;
    sha_state[3] = 0xA54FF53A;
    sha_state[4] = 0x510E527F;
    sha_state[5] = 0x9B05688C;
    sha_state[6] = 0x1F83D9AB;
    sha_state[7] = 0x5BE0CD19;

    mmcau_sha256(sha_state, data, digest, (unsigned int) len);
#else
    sha256_context_t c;
    static unsigned char m[SHA256_DIGEST_LENGTH];

    if (digest == NULL) {
        digest = m;
    }

    sha256_init(&c);
    sha256_update(&c, data, len);
    sha256_final(&c, digest);
#endif
    return digest;
}

void hmac_sha256_init(hmac_context_t *ctx, const void *key, size_t key_length)
{
    unsigned char k[SHA256_INTERNAL_BLOCK_SIZE];

    memset((void *)k, 0x00, SHA256_INTERNAL_BLOCK_SIZE);

    if (key_length > SHA256_INTERNAL_BLOCK_SIZE) {
        sha256(key, key_length, k);
    }
    else {
        memcpy((void *)k, key, key_length);
    }

    /*
     * create the inner and outer keypads
     * rising hamming distance enforcing i_* and o_* are distinct
     * in at least one bit
     */
    unsigned char o_key_pad[SHA256_INTERNAL_BLOCK_SIZE];
    unsigned char i_key_pad[SHA256_INTERNAL_BLOCK_SIZE];

    for (size_t i = 0; i < SHA256_INTERNAL_BLOCK_SIZE; ++i) {
        o_key_pad[i] = 0x5c ^ k[i];
        i_key_pad[i] = 0x36 ^ k[i];
    }

    /*
     * Initiate calculation of the inner hash
     * tmp = hash(i_key_pad CONCAT message)
     */
    sha256_init(&ctx->c_in);
    sha256_update(&ctx->c_in, i_key_pad, SHA256_INTERNAL_BLOCK_SIZE);

    /*
     * Initiate calculation of the outer hash
     * result = hash(o_key_pad CONCAT tmp)
     */
    sha256_init(&ctx->c_out);
    sha256_update(&ctx->c_out, o_key_pad, SHA256_INTERNAL_BLOCK_SIZE);

}

void hmac_sha256_update(hmac_context_t *ctx, const void *data, size_t len)
{
    sha256_update(&ctx->c_in, data, len);
}

void hmac_sha256_final(hmac_context_t *ctx, void *digest)
{
    unsigned char tmp[SHA256_DIGEST_LENGTH];

    static unsigned char m[SHA256_DIGEST_LENGTH];

    if (digest == NULL) {
        digest = m;
    }

    sha256_final(&ctx->c_in, tmp);
    sha256_update(&ctx->c_out, tmp, SHA256_DIGEST_LENGTH);
    sha256_final(&ctx->c_out, digest);
}

const void *hmac_sha256(const void *key, size_t key_length,
                        const void *data, size_t len, void *digest)
{

    hmac_context_t ctx;

    hmac_sha256_init(&ctx, key, key_length);
    hmac_sha256_update(&ctx,data, len);
    hmac_sha256_final(&ctx, digest);

    return digest;
}

/**
 * @brief helper to compute sha256 inplace for the given buffer
 *
 * @param[in, out] element the buffer to compute a sha256 and store it back to it
 *
 */
static inline void sha256_inplace(unsigned char element[SHA256_DIGEST_LENGTH])
{
    sha256_context_t ctx;

    sha256_init(&ctx);
    sha256_update(&ctx, element, SHA256_DIGEST_LENGTH);
    sha256_final(&ctx, element);
}

void *sha256_chain(const void *seed, size_t seed_length,
                   size_t elements, void *tail_element)
{
    unsigned char tmp_element[SHA256_DIGEST_LENGTH];

    /* assert if no sha256-chain can be created */
    assert(elements >= 2);

    /* 1st iteration */
    sha256(seed, seed_length, tmp_element);

    /* perform consecutive iterations minus the first one */
    for (size_t i = 0; i < (elements - 1); ++i) {
        sha256_inplace(tmp_element);
    }

    /* store the result */
    memcpy(tail_element, tmp_element, SHA256_DIGEST_LENGTH);

    return tail_element;
}

void *sha256_chain_with_waypoints(const void *seed,
                                  size_t seed_length,
                                  size_t elements,
                                  void *tail_element,
                                  sha256_chain_idx_elm_t *waypoints,
                                  size_t *waypoints_length)
{
    /* assert if no sha256-chain can be created */
    assert(elements >= 2);

    /* assert to prevent division by 0 */
    assert(*waypoints_length > 0);

    /* assert if no waypoints can be created */
    assert(*waypoints_length > 1);

    /* if we have enough space we store the whole chain */
    if (*waypoints_length >= elements) {
        /* 1st iteration */
        sha256(seed, seed_length, waypoints[0].element);
        waypoints[0].index = 0;

        /* perform consecutive iterations starting at index 1*/
        for (size_t i = 1; i < elements; ++i) {
            sha256_context_t ctx;
            sha256_init(&ctx);
            sha256_update(&ctx, waypoints[(i - 1)].element, SHA256_DIGEST_LENGTH);
            sha256_final(&ctx, waypoints[i].element);
            waypoints[i].index = i;
        }

        /* store the result */
        memcpy(tail_element, waypoints[(elements - 1)].element, SHA256_DIGEST_LENGTH);
        *waypoints_length = (elements - 1);

        return tail_element;
    }
    else {
        unsigned char tmp_element[SHA256_DIGEST_LENGTH];
        size_t waypoint_streak = (elements / *waypoints_length);

        /* 1st waypoint iteration */
        sha256(seed, seed_length, tmp_element);
        for (size_t i = 1; i < waypoint_streak; ++i) {
            sha256_inplace(tmp_element);
        }
        memcpy(waypoints[0].element, tmp_element, SHA256_DIGEST_LENGTH);
        waypoints[0].index = (waypoint_streak - 1);

        /* index of the current computed element in the chain */
        size_t index = (waypoint_streak - 1);

        /* consecutive waypoint iterations */
        size_t j = 1;
        for (; j < *waypoints_length; ++j) {
            for (size_t i = 0; i < waypoint_streak; ++i) {
                sha256_inplace(tmp_element);
                index++;
            }
            memcpy(waypoints[j].element, tmp_element, SHA256_DIGEST_LENGTH);
            waypoints[j].index = index;
        }

        /* store/pass the last used index in the waypoint array */
        *waypoints_length = (j - 1);

        /* remaining iterations down to elements */
        for (size_t i = index; i < (elements - 1); ++i) {
            sha256_inplace(tmp_element);
        }

        /* store the result */
        memcpy(tail_element, tmp_element, SHA256_DIGEST_LENGTH);

        return tail_element;
    }
}

int sha256_chain_verify_element(void *element,
                                size_t element_index,
                                void *tail_element,
                                size_t chain_length)
{
    unsigned char tmp_element[SHA256_DIGEST_LENGTH];

    int delta_count = (chain_length - element_index);

    /* assert if we have an index mismatch */
    assert(delta_count >= 1);

    memcpy((void *)tmp_element, element, SHA256_DIGEST_LENGTH);

    /* perform all consecutive iterations down to tail_element */
    for (int i = 0; i < (delta_count - 1); ++i) {
        sha256_inplace(tmp_element);
    }

    /* return if the computed element equals the tail_element */
    return (memcmp(tmp_element, tail_element, SHA256_DIGEST_LENGTH) != 0);
}
