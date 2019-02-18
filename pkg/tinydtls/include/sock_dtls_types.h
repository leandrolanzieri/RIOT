#include "event.h"
#include "net/sock/udp.h"
#include "dtls.h"

#ifndef SOCK_DTLS_MBOX_SIZE
#define SOCK_DTLS_MBOX_SIZE     (8)
#endif


typedef int (*psk_hint_storage_t)(struct sock_dtls *sock,
                                 struct sock_dtls_session *session,
                                 unsigned char *hint, size_t hint_len);

typedef int (*psk_id_storage_t)(struct sock_dtls *sock,
                                struct sock_dtls_session *session,
                                const unsigned char *hint, size_t hint_len,
                                unsigned char *id, size_t id_len);

typedef int (*psk_key_storage_t)(struct sock_dtls *sock,
                                 struct sock_dtls_session *session,
                                 const unsigned char *id, size_t id_len,
                                 unsigned char *key, size_t key_len);

typedef struct {
    session_t session;
    uint8_t *buf;
    size_t len;
} recv_msg_t;

typedef struct {
    psk_hint_storage_t psk_hint_storage;
    psk_id_storage_t psk_id_storage;
    psk_key_storage_t psk_key_storage;
} psk_keys_t;

struct sock_dtls {
    dtls_context_t *dtls_ctx;
    sock_udp_t *udp_sock;
    mbox_t mbox;
    msg_t mbox_queue[SOCK_DTLS_MBOX_SIZE];
    struct sock_dtls_session *dtls_session;
    struct sock_dtls_queue *dtls_queue;
    recv_msg_t recv_msg;
    psk_keys_t psk;
};

struct sock_dtls_session {
    session_t dtls_session;
    sock_udp_ep_t *remote_ep;
};

struct sock_dtls_queue {
    struct sock_dtls_session *sessions;
    unsigned sessions_numof;
    unsigned sessions_used;
};