/*
 * Copyright (C) 2021 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
/**
 * @{
 * @ingroup     lwm2m_objects_oscore
 *
 * @file
 * @brief       OSCORE object implementation for LwM2M client using Wakaama
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include "liblwm2m.h"
#include "objects/oscore.h"
#include "oscore.h"
#include "lwm2m_client_config.h"
#include "lwm2m_client.h"
#include "kernel_defines.h"
#include "mutex.h"
#include "od.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ENABLE_DEBUG    0
#include "debug.h"

typedef struct _key_byte_array {
    uint8_t buf[CONFIG_LWM2M_OSCORE_OBJ_BUF_SIZE];
    size_t len;
} _key_byte_array_t;

/**
 * @brief Descriptor of a LwM2M OSCORE object instance (Object ID = 21)
 */
typedef struct lwm2m_obj_oscore_inst {
    /**
     * @brief Linked list handle.
     */
    lwm2m_list_t list;

    struct context oscore_server_ctx;
    struct context oscore_client_ctx;
    _key_byte_array_t master_secret;
    _key_byte_array_t master_salt;
    _key_byte_array_t sender_id;
    _key_byte_array_t recipient_id;
    _key_byte_array_t id_ctx;
    aead_alg_t aead_alg;
    hmac_alg_t hmac_alg;
    bool used;
} lwm2m_obj_oscore_inst_t;

/**
 * @brief 'Read' callback for the OSCORE object.
 *
 * @param[in] instance_id       ID of the instance to read
 * @param[in, out] num_data     Number of resources requested. 0 means all.
 * @param[in, out] data_array   Initialized data array to output the values,
 *                              when @p num_data != 0. Uninitialized otherwise.
 * @param[in] object            OSCORE object pointer
 *
 * @retval COAP_205_CONTENT                 on success
 * @retval COAP_404_NOT_FOUND               when resource can't be found
 * @retval COAP_500_INTERNAL_SERVER_ERROR   otherwise
 */
static uint8_t _read_cb(uint16_t instance_id, int *num_data, lwm2m_data_t **data_array,
                        lwm2m_object_t *object);

/**
 * @brief Get a value from a OSCORE object instance.
 *
 * @param data[in, out]     Data structure indicating the id of the resource
 *                          to get the value of. It will contain the value if
 *                          successful.
 * @param instance[in]      Instance to get the data from.
 * @retval 0 on success
 * @retval <0 otherwise
 */
static int _get_value(lwm2m_data_t *data, lwm2m_obj_oscore_inst_t *instance);

/**
 * @brief Pool of object instances.
 */
static lwm2m_obj_oscore_inst_t _instances[CONFIG_LWM2M_OBJ_OSCORE_INSTANCES_MAX] = { 0 };

/**
 * @brief Mutex for instance pool access.
 */
static mutex_t _mutex = MUTEX_INIT;

/**
 * @brief Try to find a free instance in the pool.
 *
 * @return Pointer to the instance
 * @retval NULL if none is free
 */
static lwm2m_obj_oscore_inst_t *_get_free_instance(void)
{
    lwm2m_obj_oscore_inst_t *instance = NULL;
    mutex_lock(&_mutex);

    for (unsigned i = 0; i < CONFIG_LWM2M_OBJ_OSCORE_INSTANCES_MAX; i++) {
        /* we use the server short_id to check if the instance is not used, as 0 is not allowed */
        if (!_instances[i].used) {
            instance = &_instances[i];
            memset(instance, 0, sizeof(lwm2m_obj_oscore_inst_t));
            instance->used = true;
            break;
        }
    }

    mutex_unlock(&_mutex);
    return instance;
}

/**
 * @brief Free an instance from the pool.
 *
 * @param[in] instance      Pointer to the instance to free.
 */
static void _free_instance(lwm2m_obj_oscore_inst_t *instance)
{
    assert(instance);

    mutex_lock(&_mutex);
    instance->used = false;
    mutex_unlock(&_mutex);
}

/**
 * @brief Implementation of the object interface for the OSCORE Object.
 */
static lwm2m_object_t _oscore_object = {
    .next           = NULL,
    .objID          = LWM2M_OSCORE_OBJECT_ID,
    .instanceList   = NULL,
    .readFunc       = _read_cb,
    .writeFunc      = NULL,
    .createFunc     = NULL,
    .deleteFunc     = NULL,
    .executeFunc    = NULL,
    .discoverFunc   = NULL,
    .userData       = NULL,
};

static int _get_value(lwm2m_data_t *data, lwm2m_obj_oscore_inst_t *instance)
{
    assert(data);
    assert(instance);

    switch (data->id) {
        case LWM2M_OSCORE_MASTER_SECRET_ID:
            lwm2m_data_encode_opaque(instance->master_secret.buf, instance->master_secret.len, data);
            break;

        case LWM2M_OSCORE_MASTER_SALT_ID:
            lwm2m_data_encode_opaque(instance->master_salt.buf, instance->master_salt.len, data);
            break;

        case LWM2M_OSCORE_SENDER_ID:
            lwm2m_data_encode_opaque(instance->sender_id.buf, instance->sender_id.len, data);
            break;

        case LWM2M_OSCORE_RECIPIENT_ID:
            lwm2m_data_encode_opaque(instance->recipient_id.buf, instance->recipient_id.len, data);
            break;

        case LWM2M_OSCORE_ID_CONTEXT_ID:
            lwm2m_data_encode_opaque(instance->id_ctx.buf, instance->id_ctx.len, data);
            break;

        case LWM2M_OSCORE_AEAD_ALG_ID:
            lwm2m_data_encode_int(instance->aead_alg, data);
            break;

        case LWM2M_OSCORE_HMAC_ALG_ID:
            lwm2m_data_encode_int(instance->hmac_alg, data);
            break;

        default:
            return COAP_404_NOT_FOUND;
    }
    return COAP_205_CONTENT;
}

static uint8_t _read_cb(uint16_t instance_id, int *num_data, lwm2m_data_t **data_array,
                        lwm2m_object_t *object)
{
    lwm2m_obj_oscore_inst_t *instance;
    uint8_t result;
    int i = 0;

    /* try to get the requested instance from the object list */
    instance = (lwm2m_obj_oscore_inst_t *)LWM2M_LIST_FIND(object->instanceList, instance_id);
    if (!instance) {
        DEBUG("[lwm2m:oscore:read]: not found\n");
        return COAP_404_NOT_FOUND;
    }

    /* if the number of resources is not specified, we need to read all */
    if (!*num_data) {
        DEBUG("[lwm2m:oscore:read]: all resource are read\n");

        uint16_t resList[] = {
            LWM2M_OSCORE_MASTER_SECRET_ID,
            LWM2M_OSCORE_SENDER_ID,
            LWM2M_OSCORE_RECIPIENT_ID,
            LWM2M_OSCORE_AEAD_ALG_ID,
            LWM2M_OSCORE_HMAC_ALG_ID,
            LWM2M_OSCORE_MASTER_SALT_ID,
            LWM2M_OSCORE_ID_CONTEXT_ID,
        };

        /* try to allocate data structures for all resources */
        int resNum = ARRAY_SIZE(resList);
        *data_array = lwm2m_data_new(resNum);

        if (NULL == data_array) {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }

        /* indicate the number of resources that are returned */
        *num_data = resNum;

        /* prepare the resource ID of all resources for the request */
        for (i = 0 ; i < resNum ; i++) {
            (*data_array)[i].id = resList[i];
        }
    }

    /* the data structures in data_array contain the IDs of the resources to get the values of */
    i = 0;
    do {
        DEBUG("[security:read] read: %d\n", (*data_array)[i].id);
        result = _get_value(&(*data_array)[i], instance);
        i++;
    } while (i < *num_data && COAP_205_CONTENT == result);

    return result;
}

lwm2m_object_t *lwm2m_object_oscore_get(void)
{
    return &_oscore_object;
}

struct context *lwm2m_object_oscore_get_server_ctx(lwm2m_object_t *object, uint16_t instance_id) {
    assert(object);
    lwm2m_obj_oscore_inst_t *instance = NULL;

    /* try to get the requested instance from the object list */
    instance = (lwm2m_obj_oscore_inst_t *)LWM2M_LIST_FIND(object->instanceList, instance_id);
    if (!instance) {
        DEBUG("[lwm2m:oscore:get_ctx]: not found\n");
        return NULL;
    }

    return &instance->oscore_server_ctx;
}

struct context *lwm2m_object_oscore_get_client_ctx(lwm2m_object_t *object, uint16_t instance_id) {
    assert(object);
    lwm2m_obj_oscore_inst_t *instance = NULL;

    /* try to get the requested instance from the object list */
    instance = (lwm2m_obj_oscore_inst_t *)LWM2M_LIST_FIND(object->instanceList, instance_id);
    if (!instance) {
        DEBUG("[lwm2m:oscore:get_ctx]: not found\n");
        return NULL;
    }

    return &instance->oscore_client_ctx;
}

int lwm2m_object_oscore_instance_create(lwm2m_object_t *object, uint16_t instance_id,
                                        const lwm2m_obj_oscore_args_t *args)
{
    assert(object);
    assert(args);

    lwm2m_obj_oscore_inst_t *instance = NULL;

    /* some checks */
        if (!object || !args || !args->master_secret || !args->master_salt ||
            args->master_secret_len > CONFIG_LWM2M_OSCORE_OBJ_BUF_SIZE ||
            args->master_salt_len > CONFIG_LWM2M_OSCORE_OBJ_BUF_SIZE ||
            args->sender_id_len > CONFIG_LWM2M_OSCORE_OBJ_BUF_SIZE ||
            args->recipient_id_len > CONFIG_LWM2M_OSCORE_OBJ_BUF_SIZE ||
            args->id_ctx_len > CONFIG_LWM2M_OSCORE_OBJ_BUF_SIZE ||
            object->objID != LWM2M_OSCORE_OBJECT_ID) {
        DEBUG("[lwm2m:oscore]: invalid parameters\n");
        goto out;
    }

    /* try to allocate an instance */
    instance = _get_free_instance();
    if (!instance) {
        DEBUG("lwm2m:oscore]: can't allocate new instance\n");
        goto out;
    }
    DEBUG("[lwm2m:oscore]: creating new instance\n");
    instance->list.id = instance_id;
    instance->aead_alg = args->aead_algorithm;
    instance->hmac_alg= args->hmac_algorithm;

    /* copy all keys */
    memcpy(instance->master_salt.buf, args->master_salt, args->master_salt_len);
    instance->master_salt.len = args->master_salt_len;
    DEBUG("[lwm2m:oscore]: Master salt:");
    if (ENABLE_DEBUG) {
        od_hex_dump(instance->master_salt.buf, instance->master_salt.len, 0);
    }

    memcpy(instance->master_secret.buf, args->master_secret, args->master_secret_len);
    instance->master_secret.len = args->master_secret_len;
    DEBUG("[lwm2m:oscore]: Master secret:");
    if (ENABLE_DEBUG) {
        od_hex_dump(instance->master_secret.buf, instance->master_secret.len, 0);
    }

    if (args->sender_id) {
        memcpy(instance->sender_id.buf, args->sender_id, args->sender_id_len);
        instance->sender_id.len = args->sender_id_len;
        DEBUG("[lwm2m:oscore]: Sender ID:");
        if (ENABLE_DEBUG) {
            od_hex_dump(instance->sender_id.buf, instance->sender_id.len, 0);
        }
    }

    if (args->recipient_id) {
        memcpy(instance->recipient_id.buf, args->recipient_id, args->recipient_id_len);
        instance->recipient_id.len = args->recipient_id_len;
        DEBUG("[lwm2m:oscore]: Recipient ID:");
        if (ENABLE_DEBUG) {
            od_hex_dump(instance->recipient_id.buf, instance->recipient_id.len, 0);
        }
    }

    if (args->id_ctx) {
        memcpy(instance->id_ctx.buf, args->id_ctx, args->id_ctx_len);
        instance->id_ctx.len = args->id_ctx_len;
        DEBUG("[lwm2m:oscore]: ID context:");
        if (ENABLE_DEBUG) {
            od_hex_dump(instance->id_ctx.buf, instance->id_ctx.len, 0);
        }
    }

    struct oscore_init_params params = {
        .aead_alg = (enum AEAD_algorithm) instance->aead_alg,
        .hkdf = SHA_256,
        .dev_type = CLIENT,
        .id_context.ptr = instance->id_ctx.buf,
        .id_context.len = instance->id_ctx.len,
        .master_salt.ptr = instance->master_salt.buf,
        .master_salt.len = instance->master_salt.len,
        .master_secret.ptr = instance->master_secret.buf,
        .master_secret.len = instance->master_secret.len,
        .recipient_id.ptr = instance->recipient_id.buf,
        .recipient_id.len = instance->recipient_id.len,
        .sender_id.ptr = instance->sender_id.buf,
        .sender_id.len = instance->sender_id.len
    };

    int res = oscore_context_init(&params, &instance->oscore_client_ctx);
    if (res != OscoreNoError) {
        DEBUG("[lwm2m:oscore]: could not initialize security client context (%d)\n", res);
        goto free_out;
    }

    params.dev_type = SERVER;
    res = oscore_context_init(&params, &instance->oscore_server_ctx);
    if (res != OscoreNoError) {
        DEBUG("[lwm2m:oscore]: could not initialize security server context (%d)\n", res);
        goto free_out;
    }

    /* add the new instance to the list */
    object->instanceList = LWM2M_LIST_ADD(object->instanceList, instance);
    DEBUG("[lwm2m:oscore]: added instance with ID: %" PRId16 "\n", instance->list.id);
    goto out;

free_out:
    _free_instance(instance);
    instance = NULL;
out:
    return instance == NULL ? -1 : 0;
}
