#include <stdint.h>
#include <stdlib.h>
#include "crypto/aes.h"
#include "cryptoauthlib_crypto_hwctx.h"

int aes_encrypt_ecb(cipher_context_t *context, const uint8_t *input,
                       size_t length, uint8_t *output)
{
    (void) context;
    (void) input;
    (void) length;
    (void) output;
    return -1;
}

int aes_decrypt_ecb(cipher_context_t *context, const uint8_t *input,
                       size_t length, uint8_t *output)
{
    (void) context;
    (void) input;
    (void) length;
    (void) output;
    return -1;
}
