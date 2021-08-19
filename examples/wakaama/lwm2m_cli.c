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
 * @brief       Wakaama LwM2M Client CLI support
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include "kernel_defines.h"
#include "lwm2m_client.h"
#include "lwm2m_platform.h"
#include "objects/security.h"
#include "objects/device.h"
#include "objects/access_control.h"
#include "objects/server.h"

#include "lwm2m_obj_dump.h"
#include "net/credman.h"
#include "od.h"

#define OBJ_COUNT (7)

#ifndef CONFIG_LWM2M_SERVER_SHORT_ID
#define CONFIG_LWM2M_SERVER_SHORT_ID 1
#endif

#ifndef CONFIG_LWM2M_SERVER_URI
#define CONFIG_LWM2M_SERVER_URI "coap://[fd00:dead:beef::1]"
#endif

#ifndef CONFIG_LWM2M_CLIENT_SHORT_ID
#define CONFIG_LWM2M_CLIENT_SHORT_ID 1
#endif

#ifndef CONFIG_LWM2M_CLIENT_ENDPOINT
#define CONFIG_LWM2M_CLIENT_ENDPOINT "testRIOTDevice"
#endif

#ifndef CONFIG_LWM2M_CLIENT_URI
#define CONFIG_LWM2M_CLIENT_URI "coap://[fd00:dead:beef::1]"
#endif

uint8_t connected = 0;
lwm2m_object_t *obj_list[OBJ_COUNT];
uint8_t obj_count = 0;
lwm2m_client_data_t client_data;

#ifndef CONFIG_PSK_ID
#define CONFIG_PSK_ID "Client_Identity"
#endif

#ifndef CONFIG_PSK_KEY
#define CONFIG_PSK_KEY "ThisIsRIOT!"
#endif

#if IS_ACTIVE(CONFIG_LWM2M_CLIENT_SECURITY_PSK)
static const uint8_t client_psk_id[] = CONFIG_LWM2M_CLIENT_PSK_ID;
static const uint8_t client_psk_key[] = CONFIG_LWM2M_CLIENT_PSK_KEY;

static const credman_credential_t client_credential = {
    .type = CREDMAN_TYPE_PSK,
    .tag = CREDMAN_TAG_EMPTY,
    .params = {
        .psk = {
            .key = { .s = client_psk_key, .len = sizeof(client_psk_key) - 1, },
            .id = { .s = client_psk_id, .len = sizeof(client_psk_id) - 1, },
        }
    },
};
#endif

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

void _set_point_cb(lwm2m_object_t *object, uint16_t instance_id, uint32_t value, void *arg)
{
    (void)object;
    (void)instance_id;
    (void)arg;

    printf("Current set point value: %" PRId32 "\n", value);
}

void lwm2m_cli_init(void)
{
    lwm2m_object_t *obj = NULL;

    /* this call is needed before creating any objects */
    lwm2m_client_init(&client_data);

    /* add objects that will be registered */
    lwm2m_obj_security_args_t sec_args = {
        .server_id = CONFIG_LWM2M_SERVER_SHORT_ID,
        .server_uri = CONFIG_LWM2M_SERVER_URI,
        .security_mode = LWM2M_SECURITY_MODE_PRE_SHARED_KEY,
        .cred = &credential,
        .is_bootstrap = IS_ACTIVE(LWM2M_SERVER_IS_BOOTSTRAP), /* set to true when using Bootstrap server */
        .client_hold_off_time = 5,
        .bootstrap_account_timeout = 0,
        .oscore_object_inst_id = LWM2M_MAX_ID,
    };

    obj = lwm2m_object_security_get();
    int res = lwm2m_object_security_instance_create(obj, 0, &sec_args);
    obj_list[obj_count++] = obj;
    if (res < 0) {
        puts("Could not instantiate the security object");
        return;
    }

    obj = lwm2m_object_server_get();
    lwm2m_obj_server_args_t server_args = {
        .short_id = CONFIG_LWM2M_SERVER_SHORT_ID, /* must match the one use in the security */
        .binding = BINDING_U,
        .lifetime = 120, /* two minutes */
        .max_period = 120,
        .min_period = 60,
        .notification_storing = false,
        .disable_timeout = 3600 /* one hour */
    };

    res = lwm2m_object_server_instance_create(obj, 0, &server_args);
    obj_list[obj_count++] = obj;
    if (res < 0) {
        puts("Could not instantiate the server object");
        return;
    }

    /* device object has a single instance. All the information for now is defined at
     * compile-time */
    obj_list[obj_count++] = lwm2m_object_device_get();

    /* allow the server to create client object instances */
    obj = lwm2m_object_access_control_get();
    uint16_t acc_ctrl_inst = 0;
    lwm2m_obj_access_control_args_t acc_args = {
        .owner = LWM2M_MAX_ID,
        .obj_id = LWM2M_CLIENT_OBJECT_ID,
        .obj_inst_id = LWM2M_MAX_ID
    };
    res = lwm2m_object_access_control_instance_create(obj, acc_ctrl_inst, &acc_args);
    if (res < 0) {
        puts("Error instantiating access control");
    }

    lwm2m_obj_access_control_acl_args_t acl_args = {
        .access = LWM2M_ACCESS_CONTROL_CREATE,
        .res_inst_id = CONFIG_LWM2M_SERVER_SHORT_ID
    };
    res = lwm2m_object_access_control_add(obj, acc_ctrl_inst, &acl_args);
    obj_list[obj_count++] = obj;
    if (res < 0) {
        puts("Error instantiating ACLs");
    }

    obj = lwm2m_object_client_get();
    lwm2m_obj_client_args_t client_args = {
        .short_id = CONFIG_LWM2M_CLIENT_SHORT_ID,
        .binding = BINDING_U,
        .lifetime = 120, /* two minutes */
        .max_period = 120,
        .min_period = 60,
        .notification_storing = false,
        .disable_timeout = 3600, /* one hour */
        .endpoint = CONFIG_LWM2M_CLIENT_ENDPOINT
    };
    res = lwm2m_object_client_instance_create(obj, 0, &client_args);
    obj_list[obj_count++] = obj;
    if (res < 0) {
        puts("Error instantiating client object");
    }

    obj = lwm2m_object_client_security_get();
    lwm2m_obj_client_security_args_t client_sec_args = {
        .server_id = CONFIG_LWM2M_CLIENT_SHORT_ID,
        .server_uri = CONFIG_LWM2M_CLIENT_URI,
#if IS_ACTIVE(CONFIG_LWM2M_CLIENT_SECURITY_PSK)
        .security_mode = LWM2M_SECURITY_MODE_PRE_SHARED_KEY,
        .cred = &client_credential,
#else
        .security_mode = LWM2M_SECURITY_MODE_NONE,
        .cred = NULL,
#endif
        .is_bootstrap = false,
        .client_hold_off_time = 5,
        .bootstrap_account_timeout = 0,
        .oscore_object_inst_id = LWM2M_MAX_ID,
    };
    res = lwm2m_object_client_security_instance_create(obj, 0, &client_sec_args);
    obj_list[obj_count++] = obj;
    if (res < 0) {
        puts("Error instantiating client security object");
    }

    obj_list[obj_count++] = lwm2m_object_client_access_control_get();

    if (obj_count > OBJ_COUNT) {
        puts("Error: Too many objects instantiated");
    }
}

void _read_cb(uint16_t client_id, lwm2m_uri_t *uri, int status, lwm2m_media_type_t format,
              uint8_t *data, int data_len, void *user_data)
{
    (void) uri;
    (void) format;
    (void) user_data;

    printf("Got response from client %d. Status %d\n", client_id, status);
    od_hex_dump_ext(data, (size_t)data_len, 0, 0);
}

void _auth_cb (uint16_t short_server_id, uint8_t response_code, void *user_data)
{
    (void) user_data;

    printf("Got response from server %d: %d\n", short_server_id, response_code);
}

int lwm2m_cli_cmd(int argc, char **argv)
{
    if (argc == 1) {
        goto help_error;
    }

    if (!strcmp(argv[1], "start")) {
        /* run the LwM2M client */
        if (!connected && lwm2m_client_run(&client_data, obj_list, obj_count)) {
            connected = 1;
        }
        return 0;
    }

    if (!strcmp(argv[1], "read")) {
        if (argc != 4) {
            goto read_usage_error;
        }

        lwm2m_uri_t uri;
        if (!lwm2m_stringToUri(argv[3], strlen(argv[3]), &uri)) {
            printf("Invalid path\n");
            goto read_usage_error;
        }

        int client_id = atoi(argv[2]);

        int res = lwm2m_client_read(&client_data, client_id, &uri, _read_cb);
        if (res != COAP_231_CONTINUE) {
            printf("Error reading from client\n");
            return 1;
        }

        return 0;

read_usage_error:
        printf("usage: %s read <client_id> <path>\n", argv[0]);
        return 1;
    }

    if (!strcmp(argv[1], "obs")) {
        if (argc != 4) {
            goto obs_usage_error;
        }

        lwm2m_uri_t uri;
        if (!lwm2m_stringToUri(argv[3], strlen(argv[3]), &uri)) {
            printf("Invalid path\n");
            goto obs_usage_error;
        }

        int client_id = atoi(argv[2]);

        int res = lwm2m_client_observe(&client_data, client_id, &uri, _read_cb);
        if (res != COAP_231_CONTINUE) {
            printf("Error observing client's resource\n");
            return 1;
        }

        return 0;

obs_usage_error:
        printf("usage: %s obs <client_id> <path>\n", argv[0]);
        return 1;
    }

    if (!strcmp(argv[1], "auth")) {
        if (argc != 7 && argc != 8) {
            printf("%d\n", argc);
            goto auth_usage_error;
        }

        int server_id = atoi(argv[2]);

        lwm2m_auth_request_t req;
        req.access = atoi(argv[4]);
        req.uri.objectId = atoi(argv[5]);
        req.uri.instanceId = atoi(argv[6]);
        req.uri.flag = LWM2M_URI_FLAG_OBJECT_ID | LWM2M_URI_FLAG_INSTANCE_ID;

        bool credentials = (argc == 8 && atoi(argv[7]));

        int res = lwm2m_request_cred_and_auth(&client_data, server_id, argv[3], strlen(argv[3]),
                                              &req, 1, credentials, _auth_cb);
        if (res != COAP_231_CONTINUE) {
            printf("Error observing client's resource\n");
            return 1;
        }

        return 0;

auth_usage_error:
        printf("usage: %s auth <server_id> <client_endpoint> <access> <object_id> <instance_id> [credentials]\n", argv[0]);
        printf("where <access> is the OR'd value of:\n");
        printf("  0x01: read\n  0x02: write\n  0x04: execute\n  0x08: delete \n  0x10: discover\n");
        return 1;
    }

    if (IS_ACTIVE(DEVELHELP) && !strcmp(argv[1], "mem")) {
        lwm2m_tlsf_status();
        return 0;
    }

    if (!strcmp(argv[1], "obj")) {
        if (!connected) {
            printf("First connect to the LwM2M server\n");
            return 1;
        }

        if (argc == 2) {
            dump_client_objects(&client_data, -1);
        }
        else if (argc == 3) {
            int id = atoi(argv[2]);
            dump_client_objects(&client_data, id);
        }
        else {
            printf("usage: %s obj [object_id]\n", argv[0]);
            return 1;
        }

        return 0;
    }

help_error:
    printf("usage: %s <start", argv[0]);

    if (IS_ACTIVE(DEVELHELP)) {
        printf("|mem");
    }

    printf("|read|obs|auth|obj>\n");

    return 1;
}
