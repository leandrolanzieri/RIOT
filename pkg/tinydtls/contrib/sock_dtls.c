#include "event/callback.h"
#include "net/sock/dtls.h"
#include "dtls.h"
#include "dtls_debug.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#define RCV_BUFFER (128)

static int _event(struct dtls_context_t *ctx, session_t *session,
                  dtls_alert_level_t level, unsigned short code);

static int _get_psk_info(struct dtls_context_t *ctx, const session_t *session,
                         dtls_credentials_type_t type,
                         const unsigned char *id, size_t id_len,
                         unsigned char *result, size_t result_length);
static int _write(struct dtls_context_t *ctx, session_t *session, uint8_t *buf,
                  size_t len);

static dtls_handler_t _dtls_handler = {
    .event = _event,
    .get_psk_info = _get_psk_info,
    .write = _write,
    // .read = _read,
};

static int _write(struct dtls_context_t *ctx, session_t *session, uint8_t *buf,
                  size_t len)
{
    sock_dtls_t *sock = (sock_dtls_t *)dtls_get_app_data(ctx);
    sock_udp_ep_t remote;

    DEBUG("Writing DTLS record to:\n");
    ipv6_addr_print((ipv6_addr_t *)&session->addr);
    DEBUG(" %d %d\n", session->port, session->ifindex);

    memcpy(&remote.addr.ipv6, &session->addr, sizeof(ipv6_addr_t));
    remote.port = session->port;
    remote.netif = session->ifindex;
    remote.family = AF_INET6;

    ssize_t res = sock_udp_send(sock->udp_sock, buf, len, &remote);
    if (res <= 0) {
        DEBUG("Error: Failed to send DTLS record: %d\n", res);
    }
    return res;
}

static int _event_connect_arg = DTLS_EVENT_CONNECT;
static int _event_connected_arg = DTLS_EVENT_CONNECTED;
static int _event_renegotiate_arg = DTLS_EVENT_RENEGOTIATE;
static event_callback_t _event_connect = { .arg = &_event_connect_arg };
static event_callback_t _event_connected = { .arg = &_event_connected_arg };
static event_callback_t _event_renegotiate = { .arg = &_event_renegotiate_arg };

static int _event(struct dtls_context_t *ctx, session_t *session,
           dtls_alert_level_t level, unsigned short code)
{
    (void)level;
    (void)session;

    sock_dtls_t *sock = dtls_get_app_data(ctx);
    switch(code) {
        case DTLS_EVENT_CONNECT:
            DEBUG("Event connect\n");
            event_post(&(sock->event_queue), (event_t *)&_event_connect);
            break;
        case DTLS_EVENT_CONNECTED:
            DEBUG("Event connected\n");
            event_post(&(sock->event_queue), (event_t *)&_event_connected);
            break;
        case DTLS_EVENT_RENEGOTIATE:
            DEBUG("Event renegotiate\n");
            event_post(&(sock->event_queue), (event_t *)&_event_renegotiate);
            break;
        default:
            return 0;
    }
    return 0;
}

#define PSK_DEFAULT_IDENTITY "Client_identity"
#define PSK_DEFAULT_KEY "secretPSK"
#define PSK_OPTIONS "i:k:"
#define PSK_ID_MAXLEN 32
#define PSK_MAXLEN 32

static unsigned char psk_id[PSK_ID_MAXLEN] = PSK_DEFAULT_IDENTITY;
static size_t psk_id_length = sizeof(PSK_DEFAULT_IDENTITY) - 1;
static unsigned char psk_key[PSK_MAXLEN] = PSK_DEFAULT_KEY;
static size_t psk_key_length = sizeof(PSK_DEFAULT_KEY) - 1;

static int _get_psk_info(struct dtls_context_t *ctx, const session_t *session,
                         dtls_credentials_type_t type,
                         const unsigned char *id, size_t id_len,
                         unsigned char *result, size_t result_length)
{
    (void)ctx;
    (void)session;

    switch(type) {
        case DTLS_PSK_IDENTITY:
            if (id_len) {
                printf("got PSK identity hint: %.*s", id_len, id);
            }
            if (result_length < psk_id_length) {
                puts("Cannot set psk_identity, buffer too small");
                return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
            }

            memcpy(result, psk_id, psk_id_length);
            return psk_id_length;
        case DTLS_PSK_KEY:
            if (id_len != psk_id_length || memcmp(psk_id, id, id_len)) {
                puts("PSK for unknown id requested");
                return dtls_alert_fatal_create(DTLS_ALERT_ILLEGAL_PARAMETER);
            }
            else if (result_length < psk_key_length) {
                puts("Cannot set PSK, buffer too small");
                return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
            }

            memcpy(result, psk_key, psk_key_length);
            return psk_key_length;
        default:
            puts("Unsupported request type");
            return 0;
    }
}

int sock_dtls_init(void)
{
    dtls_init();
    dtls_set_log_level(6);
    return 0;
}

int sock_dtls_create(sock_dtls_t *sock, sock_udp_t *udp_sock, unsigned method)
{
    (void)method;
    assert(sock && udp_sock);
    sock->udp_sock = udp_sock;
    sock->dtls_ctx = dtls_new_context(sock);
    if (!sock->dtls_ctx) {
        DEBUG("Error while getting a DTLS context\n");
        return -1;
    }
    event_queue_init(&sock->event_queue);
    dtls_set_handler(sock->dtls_ctx, &_dtls_handler);
    return 0;
}

void sock_dtls_init_server(sock_dtls_t *sock, sock_dtls_queue_t *queue,
                          sock_dtls_session_t *query_array, unsigned len)
{
    (void)sock;
    // add queue to sock and assign here?
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
    event_callback_t *event;

    DEBUG("Establishing a DTLS session\n");
    /* copy the endpoint data to the tinydtls session */
    sock->dtls_session = session;
    session->remote = ep;
    session->dtls_session.port = ep->port;
    memcpy(&session->dtls_session.addr, ep->addr.ipv6, sizeof(ipv6_addr_t));
    session->dtls_session.ifindex = 0; //TODO change to actual iface
    session->dtls_session.size = sizeof(uint8_t) * 16 + sizeof(unsigned short);

    /* start a handshake */
    if (dtls_connect(sock->dtls_ctx, &session->dtls_session) < 0) {
        DEBUG("Error establishing a session\n");
        return -1;
    }
    DEBUG("Waiting for ClientHello to be sent\n");
    /* wait for the ClientHello message to be sent */
    event = (event_callback_t *)event_wait(&sock->event_queue);
    if (*(int *)event->arg != DTLS_EVENT_CONNECT) {
        DEBUG("DTLS handshake was not started\n");
        return -1;
    }
    DEBUG("ClientHello sent, waiting for handshake\n");

    /* receive packages from sock until the session is established */
    while (!(event = (event_callback_t *)event_get(&sock->event_queue))) {
        ssize_t rcv_len = sock_udp_recv(sock->udp_sock, rcv_buffer,
                                        sizeof(rcv_buffer), SOCK_NO_TIMEOUT,
                                        &remote);
        if (rcv_len >= 0) {
            DEBUG("rcv udp\n");
            dtls_session.port = remote.port;
            dtls_session.ifindex = 0; // TODO change to actual iface
            dtls_session.size = sizeof(uint8_t) * 16 + sizeof(unsigned short);
            memcpy(&dtls_session.addr, &remote.addr.ipv6, sizeof(ipv6_addr_t));
            dtls_handle_message(sock->dtls_ctx, &dtls_session, rcv_buffer,
                                rcv_len);
        }
    }

    DEBUG("popping event\n");
    int *arg = (int *)event->arg;
    DEBUG("popped event: %d\n", *arg);

    if (*arg == DTLS_EVENT_CONNECTED) {
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

ssize_t sock_dtls_recv(sock_dtls_t *sock, sock_dtls_session_t *remote,
                       void *data, size_t max_len, uint32_t timeout)
{
    (void)sock;
    (void)remote;
    (void)data;
    (void)max_len;
    (void)timeout;
    return 0;
    // session_t session;
    // sock_udp_ep_t remote_ep;
    // ssize_t res;

    // session.size = sizeof(session.addr);

    // res = sock_udp_recv(&_app_data.sock, rcv_buffer,
    //                     sizeof(rcv_buffer), SOCK_NO_TIMEOUT, &remote_ep);
    // if (res >= 0) {
    //     DEBUG("Received a message\n");
    //     memcpy(&session.addr, &remote.addr.ipv6, sizeof(ipv6_addr_t));
    //     session.port = remote.port;
    //     session.ifindex = remote.netif;
    //     if (dtls_handle_message(_dtls_ctx, &session, rcv_buffer, (int)res)) {
    //         return -1;
    //     }
    //     // wait for event and return decrypted packet
    // }
}
