#include <stdint.h>
#include <stdlib.h>
#include "cryptoauthlib.h"
#include "crypto/aes.h"
#include "crypto/ciphers.h"
#include "cryptoauthlib_crypto_hwctx.h"

int aes_encrypt_cbc(cipher_context_t *context, uint8_t iv[16],
                       const uint8_t *input, size_t length, uint8_t *output)
{
    int status;
    atcab_nonce_load(NONCE_MODE_TARGET_TEMPKEY, (uint8_t*)&context->key, 32);
    status = atcab_aes_cbc_init(&context->cbc_context, ATCA_TEMPKEY_KEYID, 0, iv);

    for (unsigned data_block = 0; data_block < length / AES_DATA_SIZE; data_block++)
    {
        int idx=data_block * AES_DATA_SIZE;
        status = atcab_aes_cbc_encrypt_block(&context->cbc_context, &input[idx], &output[idx]);
        if(status != ATCA_SUCCESS) {
            puts("ERROR: ATCA AES ECB Encrypt failed");
            return CIPHER_ERR_ENC_FAILED;
        }
    }
    return length;
}

int aes_decrypt_cbc(cipher_context_t *context, uint8_t iv[16],
                       const uint8_t *input, size_t length, uint8_t *output)
{
    int status;
    atcab_nonce_load(NONCE_MODE_TARGET_TEMPKEY, (uint8_t*)&context->key, 32);
    status = atcab_aes_cbc_init(&context->cbc_context, ATCA_TEMPKEY_KEYID, 0, iv);

    for (unsigned data_block = 0; data_block < length / AES_DATA_SIZE; data_block++)
    {
        int idx=data_block * AES_DATA_SIZE;
        status = atcab_aes_cbc_decrypt_block(&context->cbc_context, &input[idx], &output[idx]);
        if(status != ATCA_SUCCESS) {
            puts("ERROR: ATCA AES ECB Decrypt failed");
            return CIPHER_ERR_ENC_FAILED;
        }
    }
    return length;
}
