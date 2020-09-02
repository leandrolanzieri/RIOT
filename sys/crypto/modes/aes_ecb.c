#include "crypto/ciphers.h"
#include "crypto/modes/ecb.h"

int aes_encrypt_ecb(cipher_context_t *context, uint8_t *input,
                       size_t length, uint8_t *output)
{
    return cipher_encrypt_ecb(context, CIPHER_AES_128, input, length, output);
}

int aes_decrypt_ecb(cipher_context_t *context, uint8_t *input,
                       size_t length, uint8_t *output)
{
    return cipher_decrypt_ecb(context, CIPHER_AES_128, input, length, output);
}