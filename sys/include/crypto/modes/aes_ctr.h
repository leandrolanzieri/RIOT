#ifndef AES_CTR_H
#define AES_CTR_H

#include "crypto/ciphers.h"

int aes_encrypt_ctr(cipher_context_t *context, uint8_t *nonce_counter,
                       uint8_t nonce_len, const uint8_t *input, size_t length,
                       uint8_t *output);
int aes_decrypt_ctr(cipher_context_t *context, uint8_t *nonce_counter,
                       uint8_t nonce_len, const uint8_t *input, size_t length,
                       uint8_t *output);

#endif /* AES_CTR */