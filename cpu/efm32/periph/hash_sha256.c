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

#include "crypto_util.h"
#include "hashes/sha256.h"
#include "sha256_hwctx.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

/* SHA-256 initialization.  Begins a SHA-256 operation. */
void sha256_init(sha256_context_t *ctx)
{
    (void) ctx;
    DEBUG("SHA256 init HW accelerated implementation\n");
}

/* Add bytes into the hash */
void sha256_update(sha256_context_t *ctx, const void *data, size_t len)
{
    CRYPTO_TypeDef* dev = crypto_acquire();
    CRYPTO_SHA_256(dev, data, len, ctx->digest);
    crypto_release(dev);
}

/*
 * SHA-256 finalization.  Pads the input data, exports the hash value,
 * and clears the context state.
 */
void sha256_final(sha256_context_t *ctx, void *dst)
{
    memcpy(dst, ctx->digest, SHA256_DIGEST_LENGTH);
}

void *sha256(const void *data, size_t len, void *digest)
{
    sha256_context_t ctx;
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, digest);
    return digest;
}


void hmac_sha256_init(hmac_context_t *ctx, const void *key, size_t key_length)
{
    (void) ctx;
    (void) key;
    (void) key_length;

}

void hmac_sha256_update(hmac_context_t *ctx, const void *data, size_t len)
{
    (void) ctx;
    (void) data;
    (void) len;
}

void hmac_sha256_final(hmac_context_t *ctx, void *digest)
{
    (void) ctx;
    (void) digest;
}

const void *hmac_sha256(const void *key, size_t key_length,
                        const void *data, size_t len, void *digest)
{

    (void) key;
    (void) key_length;
    (void) data;
    (void) len;
    (void) digest;
    return NULL;
}

/**
//  * @brief helper to compute sha256 inplace for the given buffer
//  *
//  * @param[in, out] element the buffer to compute a sha256 and store it back to it
//  *
//  */
// static inline void sha256_inplace(unsigned char element[SHA256_DIGEST_LENGTH])
// {
//     (void) element;
// }

void *sha256_chain(const void *seed, size_t seed_length,
                   size_t elements, void *tail_element)
{
    (void) seed;
    (void) seed_length;
    (void) elements;
    (void) tail_element;
    return NULL;
}

void *sha256_chain_with_waypoints(const void *seed,
                                  size_t seed_length,
                                  size_t elements,
                                  void *tail_element,
                                  sha256_chain_idx_elm_t *waypoints,
                                  size_t *waypoints_length)
{
    (void) seed;
    (void) seed_length;
    (void) elements;
    (void) tail_element;
    (void) waypoints;
    (void) waypoints_length;
    return NULL;
}

int sha256_chain_verify_element(void *element,
                                size_t element_index,
                                void *tail_element,
                                size_t chain_length)
{
    (void) element;
    (void) element_index;
    (void) tail_element;
    (void) chain_length;
    return 0;
}
