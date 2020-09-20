#include <stdint.h>
#include <stdlib.h>
#include "crypto/aes.h"
#include "crypto/ciphers.h"
#include "cryptoauthlib_crypto_hwctx.h"

// TODO: define, this is copied from NRF52
static const cipher_interface_t aes_interface = {
    AES_BLOCK_SIZE,
    AES_KEY_SIZE,
    aes_init,
    aes_encrypt,
    aes_decrypt
};
const cipher_id_t CIPHER_AES_128 = &aes_interface;

int aes_init(cipher_context_t *context, const uint8_t *key, uint8_t keySize)
{
    (void) context;
    (void) key;
    (void) keySize;
    return -1;
}

int aes_encrypt(const cipher_context_t *context, const uint8_t *plain_block,
                uint8_t *cipher_block)
{
    (void) context;
    (void) plain_block;
    (void) cipher_block;
    return -1;
}

int aes_decrypt(const cipher_context_t *context, const uint8_t *cipher_block,
                uint8_t *plain_block)
{
    (void) context;
    (void) cipher_block;
    (void) plain_block;
    return -1;
}
