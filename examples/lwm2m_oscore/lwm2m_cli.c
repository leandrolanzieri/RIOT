/*
 * Copyright (C) 2021 HAW Hamburg
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
#include "lwm2m_client_objects.h"
#include "lwm2m_platform.h"
#include "objects/security.h"
#include "objects/device.h"
#include "objects/oscore.h"

#define OBJ_COUNT                   (4)
#define OSCORE_OBJECT_INSTANCE_ID   (0)

uint8_t connected = 0;
lwm2m_object_t *obj_list[OBJ_COUNT];
lwm2m_client_data_t client_data;

static const char master_salt[] = CONFIG_OSCORE_MASTER_SALT;
static const char master_secret[] = CONFIG_OSCORE_MASTER_SECRET;
static const char recipient_id[] = CONFIG_OSCORE_RECIPIENT_ID;
static const char sender_id[] = CONFIG_OSCORE_SENDER_ID;

void lwm2m_cli_init(void)
{
    /* this call is needed before creating any objects */
    lwm2m_client_init(&client_data);

    /* add objects that will be registered */
    lwm2m_obj_security_args_t args = {
        .server_id = CONFIG_LWM2M_SERVER_SHORT_ID,
        .server_uri = CONFIG_LWM2M_SERVER_URI,
        .security_mode = LWM2M_SECURITY_MODE_NONE, /* using no-sec, only OSCORE */
        .cred = NULL,
        .is_bootstrap = false, /* set to true when using Bootstrap server */
        .client_hold_off_time = 5,
        .bootstrap_account_timeout = 0,
        .oscore_object_inst_id = OSCORE_OBJECT_INSTANCE_ID
    };

    obj_list[0] = lwm2m_object_security_get();
    int res = lwm2m_object_security_instance_create(obj_list[0], 1, &args);

    if (res < 0) {
        puts("Could not instantiate the security object");
        return;
    }

    obj_list[1] = lwm2m_client_get_server_object(&client_data, CONFIG_LWM2M_SERVER_SHORT_ID);

    /* device object has a single instance. All the information for now is defined at
     * compile-time */
    obj_list[2] = lwm2m_object_device_get();

    const lwm2m_obj_oscore_args_t oscore_args = {
        .aead_algorithm = AEAD_ALG_AES_CCM_16_64_128,
        .hmac_algorithm = HMAC_ALG_SHA_256,
        .id_ctx = NULL,
        .id_ctx_len = 0,
        .master_salt = master_salt,
        .master_salt_len = sizeof(master_salt) - 1,
        .master_secret = master_secret,
        .master_secret_len = sizeof(master_secret) - 1,
        .recipient_id = recipient_id,
        .recipient_id_len = sizeof(recipient_id) - 1,
        .sender_id = sender_id,
        .sender_id_len = sizeof(sender_id) - 1,
    };

    obj_list[3] = lwm2m_object_oscore_get();
    res = lwm2m_object_oscore_instance_create(obj_list[3], OSCORE_OBJECT_INSTANCE_ID, &oscore_args);
    if (res < 0) {
        puts("Could not instantiate the OSCORE object");
        return;
    }

    if (!obj_list[0] || !obj_list[1] || !obj_list[2] || !obj_list[3]) {
        puts("Could not create mandatory objects");
    }
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

    if (IS_ACTIVE(DEVELHELP) && !strcmp(argv[1],"mem")) {
        lwm2m_tlsf_status();
        return 0;
    }

help_error:
    if (IS_ACTIVE(DEVELHELP)) {
        printf("usage: %s <start|mem>\n", argv[0]);
    }
    else {
        printf("usage: %s <start>\n", argv[0]);
    }

    return 1;
}
