#include <stdint.h>
#include <string.h>

#include "hashes/sha1.h"
#include "em_cmu.h"
#include "em_crypto.h"

#include "xtimer.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

uint32_t t_start[2], t_stop[2];
uint32_t time[5];

void sha1_init(sha1_context *ctx)
{
    (void) ctx;
    DEBUG("SHA1 init HW accelerated implementation\n");
}

void sha1_update(sha1_context *ctx, const void *data, size_t len)
{
    (void) ctx;
    (void) data;
    (void) len;
}

// static void sha1_pad(sha1_context *s)
// {
//     (void) s;
// }

void sha1_final(sha1_context *ctx, void *digest)
{
   (void) ctx;
   (void) digest;
}

void sha1(void *digest, const void *data, size_t len)
{
    CRYPTO_SHA_1(NULL, data, (uint64_t) len, digest);
}

#define HMAC_IPAD 0x36
#define HMAC_OPAD 0x5c

void sha1_init_hmac(sha1_context *ctx, const void *key, size_t key_length)
{
    (void) ctx;
    (void) (key);
    (void) key_length;
}

void sha1_final_hmac(sha1_context *ctx, void *digest)
{
    (void) ctx;
    (void) digest;
}