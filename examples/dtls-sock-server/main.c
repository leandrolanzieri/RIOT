/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */


/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example application  for sock dtls
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>

#include "net/sock/udp.h"
#include "net/sock/dtls.h"
#include "net/ipv6/addr.h"
#include "xtimer.h"

#include "keys.h"

#ifdef DTLS_ECC
#include "client_pub_keys.h"
#endif /* DTLS_ECC */

/* TinyDTLS WARNING check */
#ifdef WITH_RIOT_SOCKETS
#error TinyDTLS is set to use sockets but the app is configured for socks.
#endif
#define DTLS_SERVER_ADDR  "fe80::783d:a4ff:fe60:e310"
#define DTLS_SERVER_PORT (20220)

/* this credential management is too implementation specific, should be
 * should be improved later on */
#ifdef DTLS_PSK
static uint8_t psk_key_0[] = PSK_DEFAULT_KEY;
static int _key_storage(sock_dtls_t *sock, sock_dtls_session_t *session,
                        const unsigned char *id, size_t id_len,
                        unsigned char *key, size_t key_len)
{
    (void)sock;
    (void)session;
    (void)id;
    (void)id_len;
    puts("Server key storage");
    if (key_len < sizeof(psk_key_0) - 1) {
        return 0;
    }
    else {
        memcpy(key, psk_key_0, sizeof(psk_key_0) - 1);
        return sizeof(psk_key_0) - 1;
    }
}
#endif /* DTLS_PSK */

#ifdef DTLS_ECC
static dtls_ecdsa_key_t cert = {
    .priv_key = ecdsa_priv_key,
    .pub_key_x = ecdsa_pub_key_x,
    .pub_key_y = ecdsa_pub_key_y,
    .curve = DTLS_ECDH_CURVE_SECP256R1
};

static int _ecdsa_storage(sock_dtls_t *sock, sock_dtls_session_t *session,
                          dtls_ecdsa_key_t **key)
{
    (void)sock;
    (void)session;
    *key = &cert;
    return 0;
}

static int _ecdsa_verify(sock_dtls_t *sock, sock_dtls_session_t *session,
                         const unsigned char *pub_x, const unsigned char *pub_y,
                         size_t key_size)
{
    (void)sock;
    (void)session;
    if (!key_size) {
        return -1;
    }
    /* just checking every byte for now */
    for (unsigned i = 0; i < key_size; i++) {
        if ((pub_x[i] != client_ecdsa_pub_key_x[i]) ||
            (pub_y[i] != client_ecdsa_pub_key_y[i])) {
            puts("Client keys do not match");
            return -1;
        }
    }
    return 0;
}

#endif /* DLTS_ECC */

int sock_dtls_init(void);

int main(void)
{
    char server_ip[] = DTLS_SERVER_ADDR;
    sock_udp_t udp_sock;
    sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
    sock_dtls_t dtls_sock = {
#ifdef DTLS_PSK
        .psk = {
            .psk_key_storage = _key_storage
        },
#endif /* DTLS_PSK */
#ifdef DTLS_ECC
        .ecdsa = {
            .ecdsa_storage = _ecdsa_storage,
            .ecdsa_verify = _ecdsa_verify
        }
#endif /* DTLS_ECC */
    };

    uint8_t rcv[512];

    sock_udp_ep_t remote = SOCK_IPV6_EP_ANY;
    sock_dtls_session_t dtls_session;
    sock_dtls_queue_t dtls_queue;

    ipv6_addr_from_str((ipv6_addr_t *)&remote.addr.ipv6, server_ip);

    local.port = DTLS_SERVER_PORT;

    puts("DTLS sock test application\n");
    xtimer_sleep(2);
    sock_dtls_init();

    sock_udp_create(&udp_sock, &local, NULL, 0);
    sock_dtls_create(&dtls_sock, &udp_sock, 0);
    sock_dtls_init_server(&dtls_sock, &dtls_queue, &dtls_session, 1);

    while (1) {
        ssize_t res = sock_dtls_recv(&dtls_sock, &dtls_session, rcv,
                                     sizeof(rcv), SOCK_NO_TIMEOUT);
        if (res > 0) {
            printf("UDP over DTLS message: %*s\n", res, rcv);
            sock_dtls_send(&dtls_sock, &dtls_session, rcv, res);
        }
        else {
            printf("Error receiving UDP over DTLS %d", res);
            break;
        }
    }
    sock_dtls_close_session(&dtls_sock);
    puts("Terminating");
    return 0;
}
