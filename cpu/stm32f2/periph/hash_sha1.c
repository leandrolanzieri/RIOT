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
 * @author      Wei Dai and others
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 */

#include <stdint.h>
#include <string.h>

#include "hashes/sha1.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

void sha1_init(sha1_context *ctx)
{
    // TBD
    (void)ctx;
    DEBUG("SHA1 init HW accelerated implementation\n");
}

void sha1_update(sha1_context *ctx, const void *data, size_t len)
{
    // TBD
    (void)ctx;
    (void)data;
    (void)len;
}

void sha1_final(sha1_context *ctx, void *digest)
{
    // TBD
    (void)ctx;
    (void)digest;
}

void sha1(void *digest, const void *data, size_t len)
{
    // TBD
    (void)digest;
    (void)data;
    (void)len;
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
