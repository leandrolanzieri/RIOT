/*******************************************************************************
*
* Copyright (c) 2015 Intel Corporation and others.
* All rights reserved. This program and the accompanying materials
* are made available under the terms of the Eclipse Public License v1.0
* and Eclipse Distribution License v1.0 which accompany this distribution.
*
* The Eclipse Public License is available at
*    http://www.eclipse.org/legal/epl-v10.html
* The Eclipse Distribution License is available at
*    http://www.eclipse.org/org/documents/edl-v10.php.
*
* Contributors:
*    David Navarro, Intel Corporation - initial API and implementation
*    Christian Renz - Please refer to git log
*    Christian Manal - Ported to RIOT OS
*
*******************************************************************************/

/*
 * Copyright (C) 2018 Beduino Master Projekt - University of Bremen
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 * @ingroup     lwm2m_client
 *
 * @file
 * @brief       Connection handle for LwM2M client implementation using Wakaama
 *
 * @author      Christian Manal <manal@uni-bremen.de>
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include <stddef.h>
#include "kernel_defines.h"
#include "net/netif.h"
#include "uri_parser.h"

#include "liblwm2m.h"
#include "net/sock/util.h"
#include "net/sock/udp.h"

#if IS_USED(MODULE_WAKAAMA_CLIENT_DTLS)
#include "net/sock/dtls.h"
#endif

#include "lwm2m_client.h"
#include "lwm2m_client_config.h"
#include "lwm2m_client_connection.h"
#include "objects/common.h"
#include "objects/security.h"

#define ENABLE_DEBUG 0
#include "debug.h"

#define URI_LENGTH 256

/**
 * @brief Creates a new connection object based on the security instance
 *        represented by @p instance_id.
 *
 * @param[in] instance_id ID number of the instance of security object
 * @param[in, out] client_data LwM2M client data
 *
 * @return Pointer to the new connection
 */
static lwm2m_client_connection_t *_connection_create(uint16_t sec_obj_inst_id,
                                                     lwm2m_client_data_t *client_data, bool client);

/**
 * @brief Sends data with a specified connection @p conn
 *
 * @param[in] conn connection to use to send data
 * @param[in] buffer data to send
 * @param[in] buffer_size size of @p buffer
 * @param[in] client_data LwM2M client data
 *
 * @return 0 on success
 * @return -1 otherwise
 */
static int _connection_send(lwm2m_client_connection_t *conn, uint8_t *buffer,
                            size_t buffer_size,
                            lwm2m_client_data_t *client_data);

/**
 * @brief Tries to find an interface in the host string. If not, it will check
 *        if there only exists one interface, and will use it
 * @param[in]  URI      parser URI
 *
 * @return  pointer to the interface to use on success
 * @retval  NULL on error
 */
static netif_t *_get_interface(uri_parser_result_t *uri);

/**
 * @brief Sets a given interface to a given UDP endpoint
 *
 * @param[out] ep           UDP endpoint
 * @param[in]  netif        Network interface to assign
 */
static void _set_interface(sock_udp_ep_t *ep, const netif_t *netif);

void *lwm2m_connect_server(uint16_t sec_obj_inst_id, void *user_data)
{
    lwm2m_client_data_t *client_data = (lwm2m_client_data_t *)user_data;
    lwm2m_client_connection_t *new_conn = NULL;

    DEBUG("[lwm2m_connect_server] Connecting to server in sec. instance %d\n", sec_obj_inst_id);

    new_conn = _connection_create(sec_obj_inst_id, client_data, false);
    if (new_conn) {
        DEBUG("[lwm2m_connect_server] Connection created\n");
        /* if the connections list is empty this is the first node, if not
         * attach to the last one */
        if (!client_data->conn_list) {
            client_data->conn_list = new_conn;
        }
        else {
            lwm2m_client_connection_t *last = client_data->conn_list;
            while (last->next != NULL) {
                last = last->next;
            }
            last->next = new_conn;
        }
    }

    return new_conn;
}

void *lwm2m_connect_client(uint16_t sec_obj_inst_id, void *user_data)
{
    lwm2m_client_data_t *client_data = (lwm2m_client_data_t *)user_data;
    lwm2m_client_connection_t *new_conn = NULL;

    DEBUG("[lwm2m_connect_client] Connecting to client in sec. instance %d\n", sec_obj_inst_id);

    new_conn = _connection_create(sec_obj_inst_id, client_data, true);
    if (new_conn) {
        DEBUG("[lwm2m_connect_client] Connection created\n");
        /* if the connections list is empty this is the first node, if not
         * attach to the last one */
        if (!client_data->client_conn_list) {
            client_data->client_conn_list = new_conn;
        }
        else {
            lwm2m_client_connection_t *last = client_data->client_conn_list;
            while (last->next != NULL) {
                last = last->next;
            }
            last->next = new_conn;
        }
    }

    return new_conn;
}

bool _close_connection(lwm2m_client_data_t *client_data, lwm2m_client_connection_t *list,
                       lwm2m_client_connection_t *conn)
{
    if (!list || !conn) {
        DEBUG("[_close_connection] List or connection empty\n");
        return false;
    }

    if (conn == list) {
        DEBUG("[_close_connection] Closing connection\n");
        list = conn->next;
        lwm2m_free(conn);
        return true;
    }
    else {
        lwm2m_client_connection_t *prev = list;

        while(prev != NULL && prev->next != conn) {
            prev = prev->next;
        }
        if (prev != NULL) {
            DEBUG("[_close_connection] Closing connection\n");
            prev->next = conn->next;

            if (conn->type == LWM2M_CLIENT_CONN_DTLS) {
                DEBUG("[_close_connection] Closing DTLS session\n");
                sock_dtls_session_destroy(&client_data->dtls_sock, &conn->session);
            }
            lwm2m_free(conn);
            return true;
        }
    }
    DEBUG("[_close_connection] Could not find connection\n");
    return false;
}

void lwm2m_close_connection(void *sessionH, void *user_data)
{
    lwm2m_client_connection_t *conn = (lwm2m_client_connection_t *) sessionH;
    lwm2m_client_data_t *client_data = (lwm2m_client_data_t *) user_data;

    DEBUG("[lwm2m_close_connection] Will try to close connection with ID %d\n", conn->sec_inst_id);
    _close_connection(client_data, client_data->conn_list, conn);
}

void lwm2m_close_client_connection(void *sessionH, void *user_data)
{
    lwm2m_client_connection_t *conn = (lwm2m_client_connection_t *) sessionH;
    lwm2m_client_data_t *client_data = (lwm2m_client_data_t *) user_data;

    DEBUG("[lwm2m_close_client_connection] Closing client connection with ID %d\n", conn->sec_inst_id);
    _close_connection(client_data, client_data->client_conn_list, conn);
}

bool lwm2m_session_is_equal(void *session1, void *session2, void *user_data)
{
    (void)user_data;
    lwm2m_client_connection_t *conn_1 = (lwm2m_client_connection_t *)session1;
    lwm2m_client_connection_t *conn_2 = (lwm2m_client_connection_t *)session2;

    if (IS_ACTIVE(ENABLE_DEBUG)) {
        char ep[CONFIG_SOCK_URLPATH_MAXLEN];
        uint16_t port;

        sock_udp_ep_fmt(&conn_1->remote, ep, &port);
        DEBUG("[lwm2m_session_is_equal] S1 [%s]:%d\n", ep, port);

        sock_udp_ep_fmt(&conn_2->remote, ep, &port);
        DEBUG("[lwm2m_session_is_equal] S2 [%s]:%d\n", ep, port);
    }

    return (sock_udp_ep_equal(&conn_1->remote, &conn_2->remote) &&
            conn_1->type == conn_2->type);
}

uint8_t lwm2m_buffer_send(void *sessionH, uint8_t *buffer, size_t length,
                          void *userdata)
{
    lwm2m_client_data_t *client_data = (lwm2m_client_data_t *)userdata;
    lwm2m_client_connection_t *conn = (lwm2m_client_connection_t *)sessionH;

    if (!conn) {
        DEBUG("[lwm2m_buffer_send] Failed to send, missing connection\n");
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    if (_connection_send(conn, buffer, length, client_data)) {
        DEBUG("[lwm2m_buffer_send] Failed to send\n");
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    return COAP_NO_ERROR;
}

lwm2m_client_connection_t *lwm2m_client_connection_find(lwm2m_client_connection_t *conn_list,
                                                        const sock_udp_ep_t *remote,
                                                        lwm2m_client_connection_type_t type)
{
    char ip[128];
    uint8_t ip_len = 128;
    lwm2m_client_connection_t *conn = conn_list;

    if (!conn_list) {
        DEBUG("[lwm2m_client_connection_find] Connection list is null!\n");
        return NULL;
    }

    if (IS_ACTIVE(ENABLE_DEBUG)) {
        ipv6_addr_to_str(ip, (ipv6_addr_t *)&remote->addr.ipv6, ip_len);
        DEBUG("[lwm2m_client_connection_find] Looking for connection from [%s]:%d\n", ip, remote->port);
    }

    while(conn != NULL) {
        if (IS_ACTIVE(ENABLE_DEBUG)) {
            ipv6_addr_to_str(ip, (ipv6_addr_t *)&conn->remote.addr.ipv6, ip_len);
            DEBUG("[lwm2m_client_connection_find] Comparing to [%s]:%d\n", ip, conn->remote.port);
        }

        if (sock_udp_ep_equal(remote, &conn->remote) && conn->type == type) {
            break;
        }

        conn = conn->next;
    }
    return conn;
}

int lwm2m_connection_handle_packet(lwm2m_client_connection_t *conn, uint8_t *buffer,
                                   size_t num_bytes, lwm2m_client_data_t *client_data)
{
    lwm2m_handle_packet(client_data->lwm2m_ctx, buffer, num_bytes, conn);
    return 0;
}

static int _connection_send(lwm2m_client_connection_t *conn, uint8_t *buffer,
                            size_t buffer_size,
                            lwm2m_client_data_t *client_data)
{
    DEBUG("[_connection_send] trying to send %d bytes\n", buffer_size);
    if (conn->type == LWM2M_CLIENT_CONN_UDP) {
        ssize_t sent_bytes = sock_udp_send(&(client_data->sock), buffer,
                                        buffer_size, &(conn->remote));
        if (sent_bytes <= 0) {
            DEBUG("[_connection_send] Could not send UDP packet: %i\n", (int)sent_bytes);
            return -1;
        }
    }
#if IS_USED(MODULE_WAKAAMA_CLIENT_DTLS)
    else {
        ssize_t sent_bytes = sock_dtls_send(&client_data->dtls_sock, &conn->session, buffer,
                                            buffer_size, SOCK_NO_TIMEOUT);
        if (sent_bytes <= 0) {
            DEBUG("[_connection_send] Could not send DTLS packet: %i\n", (int)sent_bytes);
            return -1;
        }
    }
#endif /* MODULE_WAKAAMA_CLIENT_DTLS */

    conn->last_send = lwm2m_gettime();
    return 0;
}

static void _set_interface(sock_udp_ep_t *ep, const netif_t *netif)
{
    if (netif == NULL || ep == NULL) {
        return;
    }

    /* currently there is no way to assign a network interface to a sock
     * endpoint by means of a generic API, so we need to check */
    if (IS_USED(MODULE_GNRC_NETIF)) {
        const gnrc_netif_t *gnrc_netif = (gnrc_netif_t *)netif;
        ep->netif = (uint16_t)gnrc_netif->pid;
        DEBUG("[lwm2m:client] setting interface: %d\n", ep->netif);
    }
}

static netif_t *_get_interface(uri_parser_result_t *uri)
{
    netif_t *netif = NULL;

    if (!uri->zoneid) {
        DEBUG("[lwm2m:client] no interface defined in host\n");
        /* get the number of net interfaces */
        unsigned netif_numof = 0;
        while ((netif = netif_iter(netif)) != NULL) {
                netif_numof++;
        }
        /* if we only have one interface use that one */
        if (netif_numof == 1) {
            netif = netif_iter(NULL);
        }
        else {
            DEBUG("[_connection_create] No iface for link-local address\n");
        }
    }
    else {
        DEBUG("[lwm2m:client] getting interface by name\n");
        netif = netif_get_by_name_buffer(uri->zoneid, uri->zoneid_len);
    }

    return netif;
}

static lwm2m_client_connection_t *_connection_create(uint16_t sec_obj_inst_id,
                                                     lwm2m_client_data_t *client_data, bool client)
{
    lwm2m_client_connection_t *conn = NULL;
    char uri[CONFIG_LWM2M_URI_MAX_SIZE] = {0};
    char *port;
    bool is_bootstrap;
    int security_mode = -1;

    DEBUG("Creating connection\n");

    lwm2m_object_t *obj;
    if (client) {
        if (IS_ACTIVE(CONFIG_LWM2M_CLIENT_C2C)) {
            obj = lwm2m_object_client_security_get();
        }
        else {
            DEBUG("ATTENTION! Enable client-to-client module\n");
            goto out;
        }
    }
    else {
        obj = lwm2m_object_security_get();
    }

    security_mode = lwm2m_object_security_get_mode(obj, sec_obj_inst_id);
    if (security_mode < 0) {
        DEBUG("[_connection_create] Could not get security mode\n");
        goto out;
    }

    /* prepare Server URI query */
    lwm2m_uri_t resource_uri = {
        .objectId = LWM2M_SECURITY_URI_ID,
        .instanceId = sec_obj_inst_id,
        .resourceId = LWM2M_SECURITY_URI_ID,
        .flag = LWM2M_URI_FLAG_OBJECT_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_RESOURCE_ID
    };

    if (client) {
        resource_uri.objectId = LWM2M_CLIENT_SECURITY_OBJECT_ID;
    }

    int res = lwm2m_get_string(client_data, &resource_uri, uri, ARRAY_SIZE(uri));
    if (res < 0) {
        DEBUG("[_connection_create] Could not get security instance URI\n");
        goto out;
    }

    uri_parser_result_t parsed_uri;
    res = uri_parser_process_string(&parsed_uri, uri);

    if (0 != res || !parsed_uri.host) {
        DEBUG("[_connection_create] Could not parse URI schema\n");
        goto out;
    }

    resource_uri.resourceId = LWM2M_SECURITY_BOOTSTRAP_ID;
    res = lwm2m_get_bool(client_data, &resource_uri, &is_bootstrap);
    if (res < 0) {
        DEBUG("[_connection_create] Could verify if the server is bootstrap\n");
        goto out;
    }

    /* if no port specified, used the default server and BS-server ports */
    if (!parsed_uri.port) {
        if (is_bootstrap) {
            port = CONFIG_LWM2M_BSSERVER_PORT;
        }
        else {
            if (security_mode == LWM2M_SECURITY_MODE_NONE) {
                port = CONFIG_LWM2M_STANDARD_PORT;
            }
            else {
                port = CONFIG_LWM2M_DTLS_PORT;
            }
        }
    }
    else {
        port = parsed_uri.port;
    }

    DEBUG("[_connection_create] Creating connection to Host: %.*s, Port: %s\n",
          parsed_uri.ipv6addr_len, parsed_uri.ipv6addr, port);

    /* allocate new connection */
    conn = lwm2m_malloc(sizeof(lwm2m_client_connection_t));
    if (!conn) {
        DEBUG("[_connection_create] Could not allocate new connection\n");
        goto out;
    }

    conn->sec_inst_id = sec_obj_inst_id;
    conn->next = NULL;

    /* configure to any IPv6 */
    conn->remote.family = AF_INET6;
    conn->remote.netif = SOCK_ADDR_ANY_NETIF;
    conn->remote.port = atoi(port);

    if (!ipv6_addr_from_buf((ipv6_addr_t *)&conn->remote.addr.ipv6, parsed_uri.ipv6addr,
                            parsed_uri.ipv6addr_len)) {
        DEBUG("[_connection_create] IPv6 address malformed\n");
        goto free_out;
    }

    if (ipv6_addr_is_unspecified((const ipv6_addr_t *)&conn->remote.addr.ipv6)) {
        DEBUG("[_connection_create] Invalid server address ([::])\n");
        goto free_out;
    }

    netif_t *netif = _get_interface(&parsed_uri);
    if (netif == NULL) {
        DEBUG("[lwm2m:client] could not determine an interface to use\n");
        goto free_out;
    }
    else {
        _set_interface(&conn->remote, netif);
    }

#if IS_USED(MODULE_WAKAAMA_CLIENT_DTLS)
    uint8_t buf[DTLS_HANDSHAKE_BUFSIZE];

    /* TODO: add support for PSK */
    if (security_mode == LWM2M_SECURITY_MODE_PRE_SHARED_KEY) {
        conn->type = LWM2M_CLIENT_CONN_DTLS;
        DEBUG("[lwm2m:client] DTLS session init\n");
        res = sock_dtls_session_init(&client_data->dtls_sock, &conn->remote, &conn->session);
        if (res <= 0) {
            DEBUG("[lwm2m:client] could not initiate DTLS session\n");
            goto free_out;
        }

        DEBUG("[lwm2m:client] receiving DTLS handshake\n");
        res = sock_dtls_recv(&client_data->dtls_sock, &conn->session, buf, sizeof(buf), 3 * US_PER_SEC);
        if (res != -SOCK_DTLS_HANDSHAKE) {
            sock_dtls_session_destroy(&client_data->dtls_sock, &conn->session);
            DEBUG("[lwm2m:client] error creating session: %d\n", res);
            goto free_out;
        }
        DEBUG("[lwm2m:client] connection to server successful\n");
    }
    else {
        conn->type = LWM2M_CLIENT_CONN_UDP;
    }
#else
    conn->type = LWM2M_CLIENT_CONN_UDP;
#endif /* MODULE_WAKAAMA_CLIENT_DTLS */

    conn->last_send = lwm2m_gettime();
    goto out;

free_out:
    lwm2m_free(conn);
    conn = NULL;
out:
    return conn;
}
