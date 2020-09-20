#include <stdint.h>
#include <stdlib.h>
#include "crypto/aes.h"
#include "cryptoauthlib_crypto_hwctx.h"

int aes_encrypt_ctr(cipher_context_t *context, uint8_t nonce_counter[16],
                       uint8_t nonce_len, const uint8_t *input, size_t length,
                       uint8_t *output)
{
    (void) context;
    (void) nonce_counter;
    (void) nonce_len;
    (void) input;
    (void) length;
    (void) output;
    return -1;
}

int aes_decrypt_ctr(cipher_context_t *context, uint8_t nonce_counter[16],
                       uint8_t nonce_len, const uint8_t *input, size_t length,
                       uint8_t *output)
{
    (void) context;
    (void) nonce_counter;
    (void) nonce_len;
    (void) input;
    (void) length;
    (void) output;
    return -1;
}
