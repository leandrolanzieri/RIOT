#include <stdint.h>
#include <stdlib.h>
#include "cryptoauthlib.h"
#include "crypto/ciphers.h"
#include "crypto/aes.h"
#include "cryptoauthlib_crypto_hwctx.h"

int aes_encrypt_ecb(cipher_context_t *context, const uint8_t *input,
                       size_t length, uint8_t *output)
{
    int status;
    atcab_nonce_load(NONCE_MODE_TARGET_TEMPKEY, (uint8_t*)&context->key, 32);

    for (unsigned data_block = 0; data_block < length / AES_DATA_SIZE; data_block++)
    {
        status = atcab_aes_encrypt(ATCA_TEMPKEY_KEYID, 0, &input[data_block * AES_DATA_SIZE], output);
        if(status != ATCA_SUCCESS) {
            puts("ERROR: ATCA AES ECB Encrypt failed");
            return CIPHER_ERR_ENC_FAILED;
        }
    }
    return length;
}

int aes_decrypt_ecb(cipher_context_t *context, const uint8_t *input,
                       size_t length, uint8_t *output)
{
    int status;

    status = atcab_nonce_load(NONCE_MODE_TARGET_TEMPKEY, (uint8_t*)&context->key, 32);

    for (unsigned data_block = 0; data_block < length / AES_DATA_SIZE; data_block++)
    {
        status = atcab_aes_decrypt(ATCA_TEMPKEY_KEYID, 0, &input[data_block * AES_DATA_SIZE], output);
        if(status != ATCA_SUCCESS) {
            puts("ERROR: ATCA AES ECB Decrypt failed");
            return CIPHER_ERR_DEC_FAILED;
        }
    }
    return length;
}
