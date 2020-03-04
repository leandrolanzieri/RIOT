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
 *              https://github.com/linwenjian/Quadcopter_Will/blob/fd0b0d3db28ff4d2b93b3c481a0378bd0302ae57/Software/SDK1_1/platform/drivers/src/mmcau/src/mmcau_sha1_functions.c
 *
 * @author      Wei Dai and others
 * @author      Lena Boeckmann <lena.boeckmann@haw-hamburg.de>
 */

#include <stdint.h>
#include <string.h>

#include "vendor/MKW21D5.h"
#include "hashes/sha1.h"
#include "mmcau_hash_sha1.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

#define SHA1_K0  0x5a827999
#define SHA1_K20 0x6ed9eba1
#define SHA1_K40 0x8f1bbcdc
#define SHA1_K60 0xca62c1d6

void sha1_init(sha1_context *ctx)
{
    DEBUG("SHA1 init HW accelerated implementation\n");
    /* Initialize hash variables */
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xc3d2e1f0;
}

static void sha1_step(int count, int func, int* i, int constant, int *w)
{
    for (int j = 0; j < count; j++) {
            CAU->DIRECT[0] = MMCAU_2_CMDS((HASH+func), (ADRA+CA4));
            CAU->ADR_CAA = constant;
            CAU->LDR_CA[5] = w[(*i)-16];
            CAU->XOR_CA[5] = w[(*i)-14];
            CAU->XOR_CA[5] = w[(*i)-8];
            CAU->XOR_CA[5] = w[(*i)-3];
            CAU->ROTL_CA[5] = 1;
            w[(*i)++] = CAU->STR_CA[5];
            CAU->DIRECT[0] = MMCAU_2_CMDS((ADRA+CA5),(SHS));
        }
}

void sha1_update(sha1_context *ctx, const void *data, size_t len)
{
    int num_blks;
    int j;
    int w[80];

    unsigned int *da = (unsigned int*) data;

    /* The code I copied this from gets a number of 512 bit blocks (64 byte) instead of the input length. To be able to just recycle the code I convert len into  num_blks */

    if(len <= 64) {
        num_blks = 1;
    }
    else {
        num_blks = (len % 64 == 0) ? (len / 64) : ((len / 64) + 1);
    }

    /* Initialize hash variables in CAU */
    for (j = 0; j < 5; j++) {
        CAU->LDR_CA[j] = ctx->state[j];
    }

    for (int n = 0; n < num_blks; n++) {
        int i = 0;

        CAU->DIRECT[0] = MMCAU_1_CMD((MVRA+CA0));     /* a -> CAA */
        CAU->ROTL_CAA = 5;                          /* rotate 5 */

        for (j = 0; j < 16; j++) {
            w[i] = BYTEREV(da[i]);
            CAU->DIRECT[0] = MMCAU_2_CMDS((HASH+HFC), (ADRA+CA4));
            CAU->ADR_CAA = SHA1_K0;
            CAU->ADR_CAA = w[i++];
            CAU->DIRECT[0] = MMCAU_1_CMD(SHS);
        }
        for (j = 0; j < 16; j++){
            DEBUG("w[%d]: %x\n", j, w[j]);
        }
        for (j = 0; j < 5; j++){
            DEBUG("CA[%d]: %lx\n", j, CAU->STR_CA[j]);
        }

        sha1_step(4, HFC, &i, SHA1_K0, w);
        sha1_step(20, HFP, &i, SHA1_K20, w);
        sha1_step(20, HFM, &i, SHA1_K40, w);
        sha1_step(20, HFP, &i, SHA1_K60, w);

        for (j = 0; j < 5; j++) {
            CAU->ADR_CA[j] = ctx->state[j];
        }
        for (j = 4; j >= 0; j--) {
            ctx->state[j] = CAU->STR_CA[j];
        }
    }
}

void sha1_final(sha1_context *ctx, void *digest)
{
    // /* Swap byte order back */
    // for (int i = 0; i < 5; i++) {
    //     ctx->state[i] =
    //         (((ctx->state[i]) << 24) & 0xff000000)
    //         | (((ctx->state[i]) << 8) & 0x00ff0000)
    //         | (((ctx->state[i]) >> 8) & 0x0000ff00)
    //         | (((ctx->state[i]) >> 24) & 0x000000ff);
    // }

    /* Copy the content of the hash (20 characters) */
    memcpy(digest, ctx->state, 20);
}

void sha1(void *digest, const void *data, size_t len)
{
    sha1_context ctx;

    sha1_init(&ctx);
    sha1_update(&ctx, (unsigned char *) data, len);
    sha1_final(&ctx, digest);
}

void sha1_init_hmac(sha1_context *ctx, const void *key, size_t key_length)
{
    // TBD
    (void)ctx;
    (void)key;
    (void)key_length;
}

void sha1_final_hmac(sha1_context *ctx, void *digest)
{
    // TBD
    (void)ctx;
    (void)digest;
}
