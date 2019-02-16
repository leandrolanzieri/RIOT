#include "event.h"
#include "net/sock/udp.h"
#include "dtls.h"

#ifndef SOCK_DTLS_MBOX_SIZE
#define SOCK_DTLS_MBOX_SIZE     (8)
#endif

typedef struct {
    session_t session;
    uint8_t *buf;
    size_t len;
} recv_msg_t;

struct sock_dtls {
    dtls_context_t *dtls_ctx;
    sock_udp_t *udp_sock;
    mbox_t mbox;
    msg_t mbox_queue[SOCK_DTLS_MBOX_SIZE];
    struct sock_dtls_session *dtls_session;
    recv_msg_t recv_msg;
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