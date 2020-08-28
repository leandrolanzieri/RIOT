#include "crypto/ciphers.h"
#include "crypto/modes/ctr.h"

int aes_encrypt_ctr(cipher_context_t *context, uint8_t nonce_counter[16],
                       uint8_t nonce_len, const uint8_t *input, size_t length,
                       uint8_t *output)
{
     return cipher_encrypt_ctr(context, CIPHER_AES_128, nonce_counter,
                       nonce_len, input, length,
                       output);
}


int aes_decrypt_ctr(cipher_context_t *context, uint8_t nonce_counter[16],
                       uint8_t nonce_len, const uint8_t *input, size_t length,
                       uint8_t *output)
{
     return cipher_decrypt_ctr(context, CIPHER_AES_128, nonce_counter,
                       nonce_len, input, length,
                       output);
}
