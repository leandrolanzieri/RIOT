#include "event/callback.h"
#include "net/sock/dtls.h"
#include "dtls.h"
#include "dtls_debug.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#define RCV_BUFFER (128)

static int _event(struct dtls_context_t *ctx, session_t *session,
                  dtls_alert_level_t level, unsigned short code);
#ifdef DTLS_PSK
static int _get_psk_info(struct dtls_context_t *ctx, const session_t *session,
                         dtls_credentials_type_t type,
                         const unsigned char *id, size_t id_len,
                         unsigned char *result, size_t result_length);
#endif

static int _get_ecdsa_key(struct dtls_context_t *ctx, const session_t *session,
                          const dtls_ecdsa_key_t **result);

static int _verify_ecdsa_key(struct dtls_context_t *ctx,
                             const session_t *session,
                             const unsigned char *other_pub_x,
                             const unsigned char *other_pub_y,
                             size_t key_size);

static int _write(struct dtls_context_t *ctx, session_t *session, uint8_t *buf,
                  size_t len);

static int _read(struct dtls_context_t *ctx, session_t *session, uint8_t *buf,
                 size_t len);
static void _session_to_udp_ep(const session_t *session, sock_udp_ep_t *ep);
static void _udp_ep_to_session(const sock_udp_ep_t *ep, session_t *session);

static dtls_handler_t _dtls_handler = {
    .event = _event,
#ifdef DTLS_PSK
    .get_psk_info = _get_psk_info,
#endif
    .get_ecdsa_key = _get_ecdsa_key,
    .verify_ecdsa_key = _verify_ecdsa_key,
    .write = _write,
    .read = _read,
};

#define DTLS_EVENT_READ 0x01DB

static int _read(struct dtls_context_t *ctx, session_t *session, uint8_t *buf,
                 size_t len)
{
    msg_t msg = { .type = DTLS_EVENT_READ };
    sock_dtls_t *sock = dtls_get_app_data(ctx);
    int res = -1;

    DEBUG("Decrypted message arrived\n");
    if (sock->recv_msg.len < len && sock->recv_msg.buf) {
        DEBUG("Not enough place on buffer\n");
        res = -1;
    }
    else {
        sock->recv_msg.len = len;
        memcpy(sock->recv_msg.buf, buf, len);
        memcpy(&sock->recv_msg.session, session, sizeof(session_t));
    }
    mbox_put(&sock->mbox, &msg);
    return res;
}

static int _write(struct dtls_context_t *ctx, session_t *session, uint8_t *buf,
                  size_t len)
{
    sock_dtls_t *sock = (sock_dtls_t *)dtls_get_app_data(ctx);
    sock_udp_ep_t remote;

    _session_to_udp_ep(session, &remote);
    remote.family = AF_INET6;

    ssize_t res = sock_udp_send(sock->udp_sock, buf, len, &remote);
    if (res <= 0) {
        DEBUG("Error: Failed to send DTLS record: %d\n", res);
    }
    return res;
}

static int _event(struct dtls_context_t *ctx, session_t *session,
           dtls_alert_level_t level, unsigned short code)
{
    (void)level;
    (void)session;

    sock_dtls_t *sock = dtls_get_app_data(ctx);
    msg_t msg = { .type = code };
    switch(code) {
        // TODO unify cases
        case DTLS_EVENT_CONNECT:
            DEBUG("Event connect\n");
            mbox_put(&sock->mbox, &msg);
            break;
        case DTLS_EVENT_CONNECTED:
            DEBUG("Event connected\n");
            mbox_put(&sock->mbox, &msg);
            break;
        case DTLS_EVENT_RENEGOTIATE:
            DEBUG("Event renegotiate\n");
            mbox_put(&sock->mbox, &msg);
            break;
        default:
            return 0;
    }
    return 0;
}

#ifdef DTLS_PSK
static int _get_psk_info(struct dtls_context_t *ctx, const session_t *session,
                         dtls_credentials_type_t type,
                         const unsigned char *id, size_t id_len,
                         unsigned char *result, size_t result_length)
{
    (void)session;
    sock_dtls_t *sock = (sock_dtls_t *)dtls_get_app_data(ctx);
    sock_dtls_session_t _session;
    sock_udp_ep_t ep;

    _session_to_udp_ep(session, &ep);
    _session.remote_ep = &ep;
    memcpy(&_session.dtls_session, session, sizeof(session_t));
    switch(type) {
        case DTLS_PSK_HINT:
            if (sock->psk.psk_hint_storage) {
                return sock->psk.psk_hint_storage(sock, &_session, result,
                                                  result_length);
            }
            return 0;

        case DTLS_PSK_IDENTITY:
            DEBUG("psk id request\n");
            if (sock->psk.psk_id_storage) {
                return sock->psk.psk_id_storage(sock, &_session, id, id_len,
                                                result, result_length);
            }
            return 0;
        case DTLS_PSK_KEY:
            if (sock->psk.psk_key_storage) {
                return sock->psk.psk_key_storage(sock, &_session, id, id_len,
                                                 result, result_length);
            }
            return 0;
        default:
            DEBUG("Unsupported request type: %d\n", type);
            return 0;
    }
}
#endif

static int _get_ecdsa_key(struct dtls_context_t *ctx, const session_t *session,
                          const dtls_ecdsa_key_t **result)
{
    // TODO change this ecdsa key
    dtls_ecdsa_key_t *key;
    sock_dtls_t *sock = (sock_dtls_t *)dtls_get_app_data(ctx);
    sock_dtls_session_t _session;
    sock_udp_ep_t ep;
    if (sock->ecdsa.ecdsa_storage) {
        _session_to_udp_ep(session, &ep);
        _session.remote_ep = &ep;
        memcpy(&_session.dtls_session, session, sizeof(session_t));
        if (sock->ecdsa.ecdsa_storage(sock, &_session, &key) < 0) {
            DEBUG("Could not get the ECDSA key\n");
            return -1;
        }
        *result = key;
        return 0;
    }
    DEBUG("no ecdsa storage registered\n");
    return -1;
}

static int _verify_ecdsa_key(struct dtls_context_t *ctx,
                                          const session_t *session,
                                          const unsigned char *other_pub_x,
                                          const unsigned char *other_pub_y,
                                          size_t key_size)
{
    (void) ctx;
    (void) session;
    (void) other_pub_x;
    (void) other_pub_y;
    (void) key_size;

    /* TODO: As far for tinyDTLS 0.8.2 this is not used */

    return 0;
}

int sock_dtls_init(void)
{
    dtls_init();
    // TODO remove log
    dtls_set_log_level(6);
    return 0;
}

int sock_dtls_create(sock_dtls_t *sock, sock_udp_t *udp_sock, unsigned method)
{
    (void)method;
    assert(sock && udp_sock);
    sock->udp_sock = udp_sock;
    sock->dtls_ctx = dtls_new_context(sock);
    sock->dtls_queue = NULL;
    if (!sock->dtls_ctx) {
        DEBUG("Error while getting a DTLS context\n");
        return -1;
    }
    mbox_init(&sock->mbox, sock->mbox_queue, SOCK_DTLS_MBOX_SIZE);
    dtls_set_handler(sock->dtls_ctx, &_dtls_handler);
    return 0;
}

void sock_dtls_init_server(sock_dtls_t *sock, sock_dtls_queue_t *queue,
                          sock_dtls_session_t *query_array, unsigned len)
{
    sock->dtls_queue = queue;
    queue->sessions = query_array;
    queue->sessions_numof = len;
    queue->sessions_used = 0;
}


int sock_dtls_establish_session(sock_dtls_t *sock, sock_udp_ep_t *ep,
                                sock_dtls_session_t *session)
{
    uint8_t rcv_buffer[RCV_BUFFER];
    sock_udp_ep_t remote;
    session_t dtls_session;
    msg_t msg;

    DEBUG("Establishing a DTLS session\n");

    /* copy the endpoint data to the tinydtls session */
    memset(session, 0, sizeof(sock_dtls_session_t));
    sock->dtls_session = session;
    session->remote_ep = ep;
    _udp_ep_to_session(ep, &session->dtls_session);

    /* start a handshake */
    if (dtls_connect(sock->dtls_ctx, &session->dtls_session) < 0) {
        DEBUG("Error establishing a session\n");
        return -1;
    }
    DEBUG("Waiting for ClientHello to be sent\n");
    mbox_get(&sock->mbox, &msg);
    if (msg.type != DTLS_EVENT_CONNECT) {
        DEBUG("DTLS handshake was not started\n");
        return -1;

    }
    DEBUG("ClientHello sent, waiting for handshake\n");
    int udp_count = 0;
    /* receive packages from sock until the session is established */
    while (!mbox_try_get(&sock->mbox, &msg)) {
        ssize_t rcv_len = sock_udp_recv(sock->udp_sock, rcv_buffer,
                                        sizeof(rcv_buffer), SOCK_NO_TIMEOUT,
                                        &remote);
        if (rcv_len >= 0) {
            udp_count++;
            DEBUG("rcv udp n: %d\n", udp_count);
            _udp_ep_to_session(&remote, &dtls_session);
            dtls_handle_message(sock->dtls_ctx, &dtls_session, rcv_buffer,
                                rcv_len);
        }
    }

    if (msg.type == DTLS_EVENT_CONNECTED) {
        DEBUG("DTLS handshake successful\n");
        return 0;
    }
    else {
        DEBUG("DTLS handshake was not successful\n");
        return -1;
    }
}

int sock_dtls_close_session(sock_dtls_t *sock)
{
    return dtls_close(sock->dtls_ctx, &sock->dtls_session->dtls_session);
}

ssize_t sock_dtls_send(sock_dtls_t *sock, sock_dtls_session_t *remote,
                       const void *data, size_t len)
{
    assert(sock && remote && data);
    return dtls_write(sock->dtls_ctx, &remote->dtls_session, (uint8_t *)data,
                      len);
}

ssize_t sock_dtls_recv(sock_dtls_t *sock, sock_dtls_session_t *remote,
                       void *data, size_t max_len, uint32_t timeout)
{
    ssize_t res;
    sock_udp_ep_t ep;
    session_t session;
    msg_t msg;

    assert(sock && data);

    while (1) {
        res = sock_udp_recv(sock->udp_sock, data, max_len, timeout, &ep);
        if (res > 0) {
            sock->recv_msg.buf = (uint8_t *)data;
            sock->recv_msg.len = max_len;
            _udp_ep_to_session(&ep, &session);
            dtls_handle_message(sock->dtls_ctx, &session, (uint8_t *)data, res);

            if (mbox_try_get(&sock->mbox, &msg)) {
                switch(msg.type) {
                    case DTLS_EVENT_READ:
                        memcpy(data, sock->recv_msg.buf, sock->recv_msg.len);
                        if (remote) {
                            memcpy(&remote->dtls_session, &session,
                                   sizeof(session_t));
                        }
                        return sock->recv_msg.len;
                    case DTLS_EVENT_CONNECTED:
                        if (sock->dtls_queue &&
                            (sock->dtls_queue->sessions_used <
                            sock->dtls_queue->sessions_numof)) {
                            // copy the new session to the queue? Is really needed?
                            DEBUG("New incoming connection\n");
                        }
                        break;
                    case DTLS_EVENT_CONNECT:
                        DEBUG("New clienthello\n");
                        break;
                    default:
                        break;
                }
            }
        }
        else {
            DEBUG("Error receiving UDP packet: %d\n", res);
            goto error_out;
        }
    }

error_out:
    return -1;
}

int sock_dtls_destroy(sock_dtls_t *sock)
{
    dtls_free_context(sock->dtls_ctx);
    return 0;
}

static void _udp_ep_to_session(const sock_udp_ep_t *ep, session_t *session)
{
    session->port = ep->port;
    session->size = sizeof(ipv6_addr_t) + sizeof(unsigned short);
    session->ifindex = ep->netif;
    memcpy(&session->addr, &ep->addr.ipv6, sizeof(ipv6_addr_t));
}

static void _session_to_udp_ep(const session_t *session, sock_udp_ep_t *ep)
{
    ep->port = session->port;
    memcpy(&ep->addr.ipv6, &session->addr, sizeof(ipv6_addr_t));
}
