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
#include "objects/light_control.h"
#include "objects/access_control.h"
#include "objects/server.h"

#include "net/credman.h"
#include "od.h"

#ifdef LED0_ON
#define LED_COLOR   "Unknown"
#define LED_APP_TYPE "LED 0"
# define OBJ_COUNT (7)
#else
# define OBJ_COUNT (6)
#endif

#ifndef CONFIG_LWM2M_SERVER_SHORT_ID
#define CONFIG_LWM2M_SERVER_SHORT_ID 1
#endif

#ifndef CONFIG_LWM2M_SERVER_URI
#define CONFIG_LWM2M_SERVER_URI "coap://[fd00:dead:beef::1]"
#endif

#ifndef CONFIG_LWM2M_CLIENT_SHORT_ID
#define CONFIG_LWM2M_CLIENT_SHORT_ID 1
#endif

#ifndef CONFIG_LWM2M_CLIENT_URI
#define CONFIG_LWM2M_CLIENT_URI "coap://[fd00:dead:beef::1]"
#endif

uint8_t connected = 0;
lwm2m_object_t *obj_list[OBJ_COUNT];
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

#ifdef LED0_ON
void _led_cb(lwm2m_object_t *object, uint16_t instance_id, bool status, uint8_t dimmer, void *arg)
{
    (void)object;
    (void)instance_id;
    (void)arg;

    printf("Current dimmer value: %d%%\n", dimmer);

    if (status) {
        LED0_ON;
    }
    else {
        LED0_OFF;
    }
}
#endif

void lwm2m_cli_init(void)
{
    /* this call is needed before creating any objects */
    lwm2m_client_init(&client_data);

    /* add objects that will be registered */
    lwm2m_obj_security_args_t sec_args = {
        .server_id = CONFIG_LWM2M_SERVER_SHORT_ID,
        .server_uri = CONFIG_LWM2M_SERVER_URI,
        .security_mode = LWM2M_SECURITY_MODE_PRE_SHARED_KEY,
        .cred = &credential,
        .is_bootstrap = false, /* set to true when using Bootstrap server */
        .client_hold_off_time = 5,
        .bootstrap_account_timeout = 0
    };

    obj_list[0] = lwm2m_object_security_get();
    int res = lwm2m_object_security_instance_create(obj_list[0], 0, &sec_args);

    if (res < 0) {
        puts("Could not instantiate the security object");
        return;
    }

    obj_list[1] = lwm2m_object_server_get();
    lwm2m_obj_server_args_t server_args = {
        .short_id = CONFIG_LWM2M_SERVER_SHORT_ID, /* must match the one use in the security */
        .binding = BINDING_U,
        .lifetime = 120, /* two minutes */
        .max_period = 120,
        .min_period = 60,
        .notification_storing = false,
        .disable_timeout = 3600 /* one hour */
    };

    res = lwm2m_object_server_instance_create(obj_list[1], 0, &server_args);
    if (res < 0) {
        puts("Could not instantiate the server object");
        return;
    }

    /* device object has a single instance. All the information for now is defined at
     * compile-time */
    obj_list[2] = lwm2m_object_device_get();

    /* configure access to light control */
    obj_list[3] = lwm2m_object_access_control_get();
    uint16_t acc_ctrl_inst = 0;
    lwm2m_obj_access_control_args_t acc_args = {
        .owner = CONFIG_LWM2M_SERVER_SHORT_ID,
        .obj_id = LWM2M_LIGHT_CONTROL_OBJECT_ID,
        .obj_inst_id = 0
    };

    res = lwm2m_object_access_control_instance_create(obj_list[3], acc_ctrl_inst, &acc_args);
    if (res < 0) {
        puts("Error instantiating access control");
    }

    lwm2m_obj_access_control_acl_args_t acl_args = {
        .access = LWM2M_ACCESS_CONTROL_READ | LWM2M_ACCESS_CONTROL_WRITE,
        .res_inst_id = 0 /* default access */
    };
    res = lwm2m_object_access_control_add(obj_list[3], acc_ctrl_inst, &acl_args);

    obj_list[4] = lwm2m_object_client_get();
    lwm2m_obj_client_args_t client_args = {
        .short_id = CONFIG_LWM2M_CLIENT_SHORT_ID,
        .binding = BINDING_U,
        .lifetime = 120, /* two minutes */
        .max_period = 120,
        .min_period = 60,
        .notification_storing = false,
        .disable_timeout = 3600 /* one hour */
    };
    res = lwm2m_object_client_instance_create(obj_list[4], 0, &client_args);
    if (res < 0) {
        puts("Error instantiating client object");
    }

    obj_list[5] = lwm2m_object_client_security_get();
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
        .bootstrap_account_timeout = 0
    };
    res = lwm2m_object_client_security_instance_create(obj_list[5], 0, &client_sec_args);

    if (res < 0) {
        puts("Error instantiating client security object");
    }

#ifdef LED0_ON
    obj_list[6] = lwm2m_object_light_control_get();
    lwm2m_obj_light_control_args_t light_args = {
        .cb = _led_cb,
        .cb_arg = NULL,
        .color = LED_COLOR,
        .color_len = sizeof(LED_COLOR) - 1,
        .app_type = LED_APP_TYPE,
        .app_type_len = sizeof(LED_APP_TYPE) - 1
    };

    res = lwm2m_object_light_control_instance_create(obj_list[6], 0, &light_args);

    if (res < 0) {
        puts("Error instantiating light control");
    }
#endif

    if (!obj_list[0] || !obj_list[1] || !obj_list[2]) {
        puts("Could not create mandatory objects");
    }
}

void _read_cb(uint16_t client_id, lwm2m_uri_t *uri, int status, lwm2m_media_type_t format,
              uint8_t *data, int data_len, void *user_data)
{
    (void) uri;
    (void) status;
    (void) format;
    (void) user_data;

    printf("Got response from client %d\n", client_id);
    od_hex_dump_ext(data, (size_t)data_len, 0, 0);
}

int lwm2m_cli_cmd(int argc, char **argv)
{
    if (argc == 1) {
        goto help_error;
    }

    if (!strcmp(argv[1], "start")) {
        /* run the LwM2M client */
        if (!connected && lwm2m_client_run(&client_data, obj_list, ARRAY_SIZE(obj_list))) {
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

    if (IS_ACTIVE(DEVELHELP) && !strcmp(argv[1], "mem")) {
        lwm2m_tlsf_status();
        return 0;
    }

help_error:
    printf("usage: %s <start", argv[0]);

    if (IS_ACTIVE(DEVELHELP)) {
        printf("|mem");
    }

    printf("|read|obs>\n");

    return 1;
}
