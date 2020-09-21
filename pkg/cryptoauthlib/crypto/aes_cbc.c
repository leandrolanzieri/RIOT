#include <stdint.h>
#include <stdlib.h>
#include "crypto/aes.h"
#include "cryptoauthlib_crypto_hwctx.h"

int aes_encrypt_cbc(cipher_context_t *context, uint8_t iv[16],
                       const uint8_t *input, size_t length, uint8_t *output)
{
    (void) context;
    (void) iv;
    (void) input;
    (void) length;
    (void) output;
    return -1;
}

int aes_decrypt_cbc(cipher_context_t *context, uint8_t iv[16],
                       const uint8_t *input, size_t length, uint8_t *output)
{
    (void) context;
    (void) iv;
    (void) input;
    (void) length;
    (void) output;
    return -1;
}
