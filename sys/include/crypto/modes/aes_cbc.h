#ifndef AES_CBC_H
#define AES_CBC_H

#include "crypto/ciphers.h"

int aes_encrypt_cbc(cipher_context_t *context, uint8_t iv[16],
                       const uint8_t *input, size_t length, uint8_t *output);
int aes_decrypt_cbc(cipher_context_t *context, uint8_t iv[16],
                       const uint8_t *input, size_t length, uint8_t *output);

#endif /* AES_CBC */