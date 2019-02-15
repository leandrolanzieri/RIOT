#ifndef NET_SOCK_DTLS_H
#define NET_SOCK_DTLS_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#include "net/sock.h"
#include "net/sock/udp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Methods for sock_dtls_create.
 * Defines the DTLS protocol version to use.
 */
#define DTLSv1_SERVER
#define DTLSv1_CLIENT
#define DTLSv12_SERVER
#define DTLSv12_CLIENT

/**
 * Contains `sock_udp` used, keys and certificate.
 */
typedef struct sock_dtls sock_dtls_t;

/**
 * Contains security and handshake parameter of a client, etc.
 */
typedef struct sock_dtls_session sock_dtls_session_t;

/**
 * Array of all connected clients
 */
typedef struct sock_dtls_queue sock_dtls_queue_t;

/**
 * @brief Creates a new DTLS sock object
 *
 * @param[out] sock     The resulting DTLS sock object
 * @param[in] udp_sock  Existing UDP sock to be used
 * @param[in] method    Defines the method for the client or server to use.
 *
 * @return  0 on success.
 * @return value < 0 on error
 */
int sock_dtls_create(sock_dtls_t *sock, sock_udp_t *udp_sock, unsigned method);

/**
 * @brief Initialises the server to listen for incoming connections
 *
 * @param[in] sock          DTLS sock to listen to
 * @param[in] queue         The resulting sessions queue
 * @param[in] queue_array   Array of session objects
 * @param[in] len           Length of @p queue_array / Max allowed session
 *                          at any time
 */
void sock_dtls_init_server(sock_dtls_t *sock, sock_dtls_queue_t *queue,
                           sock_dtls_session_t *queue_array, unsigned len);

/**
 * @brief Establish DTLS session to a server. Execute the handshake step in DTLS.
 *
 * @param[in]  sock      DLTS sock to use
 * @param[in]  ep        Endpoint to establish session with
 * @param[out] session   The established session
 *
 * @return 0 on success
 * @return value < 0 on error
 */
int sock_dtls_establish_session(sock_dtls_t *sock, sock_udp_ep_t *ep,
                                sock_dtls_session_t *session);

/**
 * @brief Close an existing DTLS session
 *
 * @param[in] sock      DTLS session to close
 *
 * @return 0 on success.
 * @return value < 0 on error
 */
int sock_dtls_close_session(sock_dtls_t *sock);

/**
 * @brief Decrypts and reads a message from a remote peer.
 *
 * @param[in] sock      DTLS sock to use
 * @param[out] remote   DTLS session of the received data.
 *                      May be `NULL`, if it is not required by the application
 * @param[out] buf      Pointer where the data should be stored
 * @param[in] maxlen    Maximum space available at @p data
 *
 * @return The number of bytes received on success
 * @return value < 0 on error
 */
ssize_t sock_dtls_recv(sock_dtls_t *sock, sock_dtls_session_t *remote,
                       void *data, size_t max_len, uint32_t timeout);

/**
 * @brief Encrypts and sends a message to a remote peer
 *
 * @param[in] sock      DTLS sock to use
 * @param[in] remote    DTLS session to use. Must not be NULL and
 *                      the session must be already established with
 *                      `sock_dtls_establish_session()` or
 *                      use the session returned by `sock_dtls_recv`.
 * @param[in] data      Pointer where the data to be send are stored
 * @param[in] len       Length of @p data to be send
 *
 * @return The number of bytes sent on success
 * @return value < 0 on error
 */
ssize_t sock_dtls_send(sock_dtls_t *sock, sock_dtls_session_t *remote,
                       const void *data, size_t len);

/**
 * @brief Destroys DTLS sock created by `sock_dtls_create`
 *
 * @param sock          DTLS sock to destroy
 *
 * @return 0 on success
 * @return value < 0 on error
 */
int sock_dtls_destroy(sock_dtls_t *sock);

#include "sock_dtls_types.h"

#ifdef __cplusplus
}
#endif

#endif /* NET_SOCK_DTLS_H */
/** @} */
