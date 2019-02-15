#include "event.h"
#include "net/sock/udp.h"
#include "dtls.h"

struct sock_dtls {
    dtls_context_t *dtls_ctx;
    sock_udp_t *udp_sock;
    event_queue_t event_queue;
    struct sock_dtls_session *dtls_session;
};

struct sock_dtls_session {
    session_t dtls_session;
    sock_udp_ep_t *remote;
};

struct sock_dtls_queue {
    struct sock_dtls_session *sessions;
    unsigned sessions_numof;
    unsigned sessions_used;
};