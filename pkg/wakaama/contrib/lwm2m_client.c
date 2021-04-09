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
 * @brief       LwM2M client implementation using Wakaama
 *
 * @author      Christian Manal <manal@uni-bremen.de>
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include <string.h>

#include "kernel_defines.h"
#include "timex.h"
#include "liblwm2m.h"
#include "uri_parser.h"
#include "event/timeout.h"
#include "net/sock/async/event.h"
#include "net/sock/util.h"
#include "objects/common.h"
#include "objects/security.h"
#include "objects/device.h"
#include "objects/access_control.h"

#include "lwm2m_platform.h"
#include "lwm2m_client.h"
#include "lwm2m_client_config.h"
#include "lwm2m_client_connection.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/**
 * @brief   Callback for event timeout that performs a step on the LwM2M FSM.
 */
static void _lwm2m_step_cb(event_t *arg);

/**
 * @brief   Main thread of the LwM2M client that processes events.
 */
static void *_event_loop(void *arg);

/**
 * @brief   Callback to handle UDP sock events.
 */
static void _udp_event_handler(sock_udp_t *sock, sock_async_flags_t type, void *arg);

#if IS_USED(MODULE_WAKAAMA_CLIENT_DTLS)
/**
 * @brief   Callback to handle DTLS sock events.
 */
static void _dtls_event_handler(sock_dtls_t *sock, sock_async_flags_t type, void *arg);
#endif

static event_queue_t _queue;
static event_t _lwm2m_step_event = { .handler = _lwm2m_step_cb };
static event_timeout_t _lwm2m_step_event_timeout;
static lwm2m_client_data_t *_client_data = NULL;
static char _lwm2m_client_stack[2 * THREAD_STACKSIZE_MAIN + THREAD_EXTRA_STACKSIZE_PRINTF];

#if IS_USED(MODULE_WAKAAMA_CLIENT_DTLS)
/**
 * @brief Callback registered to the client DTLS sock to select a PSK credential to use.
 */
static credman_tag_t _client_psk_cb(sock_dtls_t *sock, sock_udp_ep_t *ep, credman_tag_t tags[],
                                    unsigned tags_len, const char *hint, size_t hint_len)
{
    (void) sock;
    (void) tags;
    (void) tags_len;
    (void) hint;
    (void) hint_len;

    DEBUG("[lwm2m:client:PSK] getting credential\n");

    lwm2m_object_t *sec = lwm2m_get_object_by_id(_client_data, LWM2M_SECURITY_OBJECT_ID);
    if (!sec) {
        DEBUG("[lwm2m:client:PSK] no security object found\n");
        return CREDMAN_TAG_EMPTY;
    }

    /* prepare query */
    lwm2m_uri_t query_uri = {
        .objectId = LWM2M_SECURITY_OBJECT_ID,
        .resourceId = LWM2M_SECURITY_URI_ID,
        .flag = LWM2M_URI_FLAG_OBJECT_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_RESOURCE_ID
    };

    lwm2m_list_t *instance = sec->instanceList;

    /* check all registered security object instances */
    while(instance) {
        char uri[CONFIG_LWM2M_URI_MAX_SIZE] = { 0 };
        query_uri.instanceId = instance->id;

        int res = lwm2m_get_string(_client_data, &query_uri, uri, sizeof(uri));
        if (res < 0) {
            DEBUG("[lwm2m:client:PSK] could not get URI of %d\n", instance->id);
        }
        else {
            sock_udp_ep_t inst_ep;
            uri_parser_result_t parsed_uri;
            res = uri_parser_process_string(&parsed_uri, uri);
            if (res < 0) {
                DEBUG("[lwm2m:client:PSK] could not parse URI\n");
            }

            res = sock_udp_str2ep(&inst_ep, parsed_uri.host);
            if (res < 0) {
                DEBUG("[lwm2m:client:PSK] could not convert URI to EP (%s)\n", parsed_uri.host);
            }
            else {
                if (sock_udp_ep_equal(ep, &inst_ep)) {
                    DEBUG("[lwm2m:client:PSK] found matching EP on instance %d\n", instance->id);
                    DEBUG("[lwm2m:client:PSK] tag: %d\n", lwm2m_object_security_get_credential(sec, instance->id));
                    return lwm2m_object_security_get_credential(sec, instance->id);
                }
            }
        }

        instance = instance->next;
    }

    return CREDMAN_TAG_EMPTY;
}
#endif /* MODULE_WAKAAMA_CLIENT_DTLS */

void lwm2m_client_init(lwm2m_client_data_t *client_data)
{
    (void)client_data;
    lwm2m_platform_init();
}


int lwm2m_get_access(uint16_t server_id, lwm2m_uri_t *uri, void *user_data)
{
    (void) server_id;
    (void) uri;
    lwm2m_client_data_t *client_data = (lwm2m_client_data_t *)user_data;
    lwm2m_object_t *acc_ctrl = lwm2m_get_object_by_id(client_data, LWM2M_ACCESS_CONTROL_OBJECT_ID);

    if (!acc_ctrl) {
        DEBUG("[lwm2m_client_get_access] Cant find Access Control object\n");
        return -1;
    }

    return lwm2m_object_access_control_get_access(server_id, uri, acc_ctrl);
}

lwm2m_context_t *lwm2m_client_run(lwm2m_client_data_t *client_data,
                                  lwm2m_object_t *obj_list[],
                                  uint16_t obj_numof)
{
    int res;

    _client_data = client_data;
    _client_data->local_ep.family = AF_INET6;
    _client_data->local_ep.netif = SOCK_ADDR_ANY_NETIF;

    /* create sock for UDP server */
    _client_data->local_ep.port = atoi(CONFIG_LWM2M_LOCAL_PORT);
    if (sock_udp_create(&_client_data->sock, &_client_data->local_ep, NULL, 0) < 0) {
        DEBUG("[lwm2m_client_run] Can't create server socket\n");
        return NULL;
    }

#if IS_USED(MODULE_WAKAAMA_CLIENT_DTLS)
    /* create sock for DTLS server */
    _client_data->dtls_local_ep.family = AF_INET6;
    _client_data->dtls_local_ep.netif = SOCK_ADDR_ANY_NETIF;
    _client_data->dtls_local_ep.port = atoi(CONFIG_LWM2M_LOCAL_DTLS_PORT);
    res = sock_udp_create(&_client_data->dtls_udp_sock, &_client_data->dtls_local_ep, NULL, 0);
    if (res < 0) {
        DEBUG("[lwm2m_client_run] Can't create DTLS server UDP sock\n");
        return NULL;
    }

    res = sock_dtls_create(&_client_data->dtls_sock, &_client_data->dtls_udp_sock,
                           CREDMAN_TAG_EMPTY, SOCK_DTLS_1_2, SOCK_DTLS_CLIENT);
    if (res < 0) {
        DEBUG("[lwm2m_client_run] Can't create DTLS server sock\n");
        sock_udp_close(&_client_data->dtls_udp_sock);
        sock_udp_close(&_client_data->sock);
        return NULL;
    }

    /* register callback for PSK credential selection */
    sock_dtls_set_client_psk_cb(&_client_data->dtls_sock, _client_psk_cb);
#endif /* MODULE_WAKAAMA_CLIENT_DTLS */

    /* initiate LwM2M */
    _client_data->lwm2m_ctx = lwm2m_init(_client_data);
    if (!_client_data->lwm2m_ctx) {
        DEBUG("[lwm2m_client_run] Failed to initiate LwM2M\n");
        return NULL;
    }

    res = lwm2m_configure(_client_data->lwm2m_ctx, CONFIG_LWM2M_DEVICE_NAME, NULL,
                          CONFIG_LWM2M_ALT_PATH, obj_numof, obj_list);
    if (res) {
        DEBUG("[lwm2m_client_run] Failed to configure LwM2M\n");
        return NULL;
    }
 
#if IS_USED(MODULE_WAKAAMA_CLIENT_DTLS)
    lwm2m_client_refresh_dtls_credentials();
#endif

    thread_create(_lwm2m_client_stack, sizeof(_lwm2m_client_stack), THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST, _event_loop, NULL, "lwm2m client");

    return _client_data->lwm2m_ctx;
}

static void _udp_event_handler(sock_udp_t *sock, sock_async_flags_t type, void *arg)
{
    (void) arg;

    sock_udp_ep_t remote;
    uint8_t rcv_buf[LWM2M_CLIENT_RCV_BUFFER_SIZE];

    if (type & SOCK_ASYNC_MSG_RECV) {
        ssize_t rcv_len = sock_udp_recv(sock, rcv_buf, sizeof(rcv_buf), 0, &remote);
        if (rcv_len <= 0) {
            DEBUG("[lwm2m:client] UDP receive failure: %d\n", (int)rcv_len);
            return;
        }

        DEBUG("[lwm2m:client] finding connection\n");
        lwm2m_client_connection_t *conn = lwm2m_client_connection_find(_client_data->conn_list,
                                                                       &remote);
        if (conn) {
            DEBUG("[lwm2m:client] handle packet (%i bytes)\n", (int)rcv_len);
            int result = lwm2m_connection_handle_packet(conn, rcv_buf, rcv_len, _client_data);
            if (0 != result) {
                DEBUG("[lwm2m:client] error handling message %i\n", result);
            }
        }
        else {
            DEBUG("[lwm2m:client] couldn't find incoming connection\n");
        }
    }
}

#if IS_USED(MODULE_WAKAAMA_CLIENT_DTLS)
static void _dtls_event_handler(sock_dtls_t *sock, sock_async_flags_t type, void *arg)
{
    (void) arg;

    sock_udp_ep_t remote;
    sock_dtls_session_t dtls_remote;
    uint8_t rcv_buf[LWM2M_CLIENT_RCV_BUFFER_SIZE];

    if (type & SOCK_ASYNC_MSG_RECV) {
        ssize_t rcv_len = sock_dtls_recv(sock, &dtls_remote, rcv_buf, sizeof(rcv_buf), 0);
        if (rcv_len <= 0) {
            DEBUG("[lwm2m:client] DTLS receive failure: %d\n", (int)rcv_len);
            return;
        }

        sock_dtls_session_get_udp_ep(&dtls_remote, &remote);

        DEBUG("[lwm2m:client] finding connection\n");
        lwm2m_client_connection_t *conn = lwm2m_client_connection_find(_client_data->conn_list,
                                                                       &remote);
        if (conn) {
            DEBUG("[lwm2m:client] handle packet (%i bytes)\n", (int)rcv_len);
            int result = lwm2m_connection_handle_packet(conn, rcv_buf, rcv_len, _client_data);
            if (0 != result) {
                DEBUG("[lwm2m:client] error handling message %i\n", result);
            }
        }
        else {
            DEBUG("[lwm2m:client] couldn't find incoming connection\n");
        }
    }
}
#endif /* MODULE_WAKAAMA_CLIENT_DTLS */

static void *_event_loop(void *arg)
{
    (void) arg;

    event_queue_init(&_queue);
    event_timeout_init(&_lwm2m_step_event_timeout, &_queue, &_lwm2m_step_event);
    sock_udp_event_init(&_client_data->sock, &_queue, _udp_event_handler, NULL);

#if IS_USED(MODULE_WAKAAMA_CLIENT_DTLS)
    sock_dtls_event_init(&_client_data->dtls_sock, &_queue, _dtls_event_handler, NULL);
#endif

    event_timeout_set(&_lwm2m_step_event_timeout, LWM2M_CLIENT_MIN_REFRESH_TIME);
    event_loop(&_queue);

    return 0;
}

static void _lwm2m_step_cb(event_t *arg)
{
    (void) arg;
    time_t next_step = LWM2M_CLIENT_MIN_REFRESH_TIME;

    /* check if we need to reboot */
    if (lwm2m_device_reboot_requested()) {
        DEBUG("[lwm2m:client] reboot requested, rebooting ...\n");
        pm_reboot();
    }

    /* perform step on the LwM2M FSM */
    lwm2m_step(_client_data->lwm2m_ctx, &next_step);
    DEBUG("[lwm2m:client] state: ");
    switch (_client_data->lwm2m_ctx->state) {
        case STATE_INITIAL:
            DEBUG("STATE_INITIAL\n");
            break;
        case STATE_BOOTSTRAP_REQUIRED:
            DEBUG("STATE_BOOTSTRAP_REQUIRED\n");
            break;
        case STATE_BOOTSTRAPPING:
            DEBUG("STATE_BOOTSTRAPPING\n");
            break;
        case STATE_REGISTER_REQUIRED:
            DEBUG("STATE_REGISTER_REQUIRED\n");
            break;
        case STATE_REGISTERING:
            DEBUG("STATE_REGISTERING\n");
            break;
        case STATE_READY:
            DEBUG("STATE_READY\n");
            if (next_step > LWM2M_CLIENT_MIN_REFRESH_TIME) {
                next_step = LWM2M_CLIENT_MIN_REFRESH_TIME;
            }
            break;
        default:
            DEBUG("Unknown...\n");
            break;
    }

    /* program next step */
    event_timeout_set(&_lwm2m_step_event_timeout, next_step * US_PER_SEC);
}

#if IS_USED(MODULE_WAKAAMA_CLIENT_DTLS)
void lwm2m_client_add_credential(credman_tag_t tag)
{
    if (!_client_data) {
        return;
    }

    const credman_tag_t *creds;
    size_t creds_len = sock_dtls_get_credentials(&_client_data->dtls_sock, &creds);

    DEBUG("[lwm2m:client] trying to add credential with tag %d\n", tag);
    for (unsigned i = 0; i < creds_len; i++) {
        if (creds[i] == tag) {
            return;
        }
    }

    sock_dtls_add_credential(&_client_data->dtls_sock, tag);
    DEBUG("[lwm2m:client] added\n");
}

void lwm2m_client_remove_credential(credman_tag_t tag)
{
    DEBUG("[lwm2m:client] removing credential with tag %d\n", tag);
    sock_dtls_remove_credential(&_client_data->dtls_sock, tag);
}

void lwm2m_client_refresh_dtls_credentials(void)
{
    if (!_client_data) {
        return;
    }

    DEBUG("[lwm2m:client:refresh_cred] refreshing DTLS credentials\n");

    lwm2m_object_t *sec = lwm2m_get_object_by_id(_client_data, LWM2M_SECURITY_OBJECT_ID);
    if (!sec) {
        DEBUG("[lwm2m:client:refresh_cred] no security object found\n");
        return;
    }

    /* prepare query */
    lwm2m_uri_t query_uri = {
        .objectId = LWM2M_SECURITY_OBJECT_ID,
        .flag = LWM2M_URI_FLAG_OBJECT_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_RESOURCE_ID
    };

    lwm2m_list_t *instance = sec->instanceList;
    int64_t val;

    /* check all registered security object instances */
    while(instance) {
        query_uri.instanceId = instance->id;
        /* get the security mode */
        query_uri.resourceId = LWM2M_SECURITY_SECURITY_ID;
        int res = lwm2m_get_int(_client_data, &query_uri, &val);
        if (res < 0) {
            DEBUG("[lwm2m:client:refresh_cred] could not get security mode of %d\n", instance->id);
        }
        else {
            if (val == LWM2M_SECURITY_MODE_PRE_SHARED_KEY || val == LWM2M_SECURITY_MODE_RAW_PUBLIC_KEY) {
                credman_tag_t tag = lwm2m_object_security_get_credential(sec, instance->id);
                if (tag != CREDMAN_TAG_EMPTY) {
                    lwm2m_client_add_credential(tag);
                }
            }
        }

        instance = instance->next;
    }
}
#endif /* MODULE_WAKAAMA_CLIENT_DTLS */
