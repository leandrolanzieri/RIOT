#include <stdint.h>
#include <string.h>

#include "hashes/sha256.h"
#include "sha256_hwctx.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

void sha256_init(sha256_context_t *ctx)
{
    (void) ctx;
    DEBUG("SHA-256 init Cryptoauth Lib implementation\n");
}

void sha256_update(sha256_context_t *ctx, const void *data, size_t len)
{
    (void) ctx;
    (void) data;
    (void) len;
}

void sha256_final(sha256_context_t *ctx, void *dst)
{
    (void) ctx;
    (void) dst;
}

void *sha256(const void *data, size_t len, void *digest)
{
    (void) data;
    (void) len;
    (void) digest;
    return NULL;
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
    return -1;
}
