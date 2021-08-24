#ifndef DTLS_CREDS_H
#define DTLS_CREDS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "net/credman.h"

/* configured from Kconfig */
static const uint8_t psk_id[] = CONFIG_PSK_ID;
static const uint8_t psk_key[] = CONFIG_PSK_KEY;

static const credman_credential_t credential = {
    .type = CREDMAN_TYPE_PSK,
    .tag = CREDMAN_TAG_EMPTY,
    .params = {
        .psk = {
            .key = { .s = psk_key, .len = sizeof(psk_key) - 1, },
            .id = { .s = psk_id, .len = sizeof(psk_id) - 1, },
        }
    },
};

#ifdef __cplusplus
}
#endif

#endif /* DTLS_CREDS_H */
