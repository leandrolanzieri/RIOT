/*******************************************************************************
 *
 * Copyright (c) 2015 Bosch Software Innovations GmbH Germany.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v20.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Pascal Rieux - please refer to git log
 *    Leandro Lanzieri - Adaption for RIOT OS
 *
 ******************************************************************************/

/**
 * @{
 * @ingroup     lwm2m_objects_server
 *
 * @file
 * @brief       Server object implementation for LwM2M client using Wakaama
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include <stdint.h>
#include <string.h>

#include "kernel_defines.h"
#include "objects/server.h"
#include "liblwm2m.h"

#define ENABLE_DEBUG    0
#include "debug.h"

#define MAX_BINDING_LEN     3
#define MAX_ENDPOINT_NAME   64

/**
 * @brief   Server object instance descriptor.
 */
typedef struct lwm2m_obj_server_inst {
    lwm2m_list_t list;          /**< list handle */
    uint16_t short_id;          /**< short server ID */
    uint32_t lifetime;          /**< lifetime */
    uint32_t min_period;        /**< default minimum period */
    uint32_t max_period;        /**< default maximum period */
    uint32_t disable_timeout;   /**< disable timeout */
    bool store;                 /**< notification storing */
    lwm2m_binding_t binding;    /**< binding */
    char ep[MAX_ENDPOINT_NAME]; /**< client endpoint name */
    bool client;                /**< respresents another client */
} lwm2m_obj_server_inst_t;

/**
 * @brief   'Read' callback for the server object.
 */
static uint8_t _read_cb(uint16_t instance_id, int *num_data, lwm2m_data_t **data_array,
                       lwm2m_object_t *object);

/**
 * @brief   'Delete' callback for the server object.
 */
static uint8_t _delete_cb(uint16_t instance_id, lwm2m_object_t *object);

/**
 * @brief   'Execute' callback for the server object.
 */
static uint8_t _execute_cb(uint16_t instance_id, uint16_t resource_id, uint8_t *buffer, int length,
                           lwm2m_object_t *object);

/**
 * @brief   'Write' callback for the server object.
 */
static uint8_t _write_cb(uint16_t instance_id, int num_data, lwm2m_data_t * data_array,
                         lwm2m_object_t * object);

/**
 * @brief   'Create' callback for the server object.
 */
static uint8_t _create_cb(uint16_t instance_id, int num_data, lwm2m_data_t *data_array,
                          lwm2m_object_t * object);

/**
 * @brief   'Discover' callback for the server object.
 */
static uint8_t _discover_cb(uint16_t instance_id, int *num_data, lwm2m_data_t **data_array,
                            lwm2m_object_t *object);

/**
 * @brief Transport binding strings.
 */
static const char *_bindings[] = {
    [BINDING_UNKNOWN] = "",
    [BINDING_U] = "U",
    [BINDING_UQ] = "UQ",
    [BINDING_S] = "S",
    [BINDING_SQ] = "SQ",
    [BINDING_US] = "US",
    [BINDING_UQS] = "UQS"
};

#define BINDINGS_NUMOF  ARRAY_SIZE(_bindings)

/**
 * @brief   Pool of server object instances.
 */
lwm2m_obj_server_inst_t _instances[CONFIG_LWM2M_SERVER_INSTANCES_MAX];

static lwm2m_object_t _server_object = {
    .next           = NULL,
    .objID          = LWM2M_SERVER_OBJECT_ID,
    .instanceList   = NULL,
    .readFunc       = _read_cb,
    .writeFunc      = _write_cb,
    .createFunc     = _create_cb,
    .deleteFunc     = _delete_cb,
    .executeFunc    = _execute_cb,
    .discoverFunc   = _discover_cb,
    .userData       = NULL
};

static lwm2m_object_t _client_object = {
    .next           = NULL,
    .objID          = LWM2M_CLIENT_OBJECT_ID,
    .instanceList   = NULL,
    .readFunc       = _read_cb,
    .writeFunc      = _write_cb,
    .createFunc     = _create_cb,
    .deleteFunc     = _delete_cb,
    .executeFunc    = _execute_cb,
    .discoverFunc   = _discover_cb,
    .userData       = NULL
};

static lwm2m_obj_server_inst_t *_get_free_instance(void)
{
    lwm2m_obj_server_inst_t *instance = NULL;

    for (unsigned i = 0; i < CONFIG_LWM2M_SERVER_INSTANCES_MAX; i++) {
        if (!_instances[i].short_id) {
            DEBUG("[lwm2m:server] instance %d is free\n", i);
            instance = &_instances[i];
            break;
        }
    }

    if (instance) {
        memset(instance, 0, sizeof(lwm2m_obj_server_inst_t));
    }

    return instance;
}

static void _free_instance(lwm2m_obj_server_inst_t *instance)
{
    if (!instance) {
        return;
    }
    memset(instance, 0, sizeof(lwm2m_obj_server_inst_t));
}

static uint8_t _get_resource_value(lwm2m_data_t *data, lwm2m_obj_server_inst_t *instance)
{
    switch (data->id) {
    case LWM2M_SERVER_SHORT_ID_ID:
        lwm2m_data_encode_int(instance->short_id, data);
        return COAP_205_CONTENT;

    case LWM2M_SERVER_LIFETIME_ID:
        lwm2m_data_encode_int(instance->lifetime, data);
        return COAP_205_CONTENT;

    case LWM2M_SERVER_MIN_PERIOD_ID:
        lwm2m_data_encode_int(instance->min_period, data);
        return COAP_205_CONTENT;

    case LWM2M_SERVER_MAX_PERIOD_ID:
        lwm2m_data_encode_int(instance->max_period, data);
        return COAP_205_CONTENT;

    case LWM2M_SERVER_DISABLE_ID:
        return COAP_405_METHOD_NOT_ALLOWED;

    case LWM2M_SERVER_TIMEOUT_ID:
        lwm2m_data_encode_int(instance->disable_timeout, data);
        return COAP_205_CONTENT;

    case LWM2M_SERVER_STORING_ID:
        lwm2m_data_encode_bool(instance->store, data);
        return COAP_205_CONTENT;

    case LWM2M_SERVER_BINDING_ID:
        lwm2m_data_encode_string(_bindings[instance->binding], data);
        return COAP_205_CONTENT;

    case LWM2M_SERVER_UPDATE_ID:
        return COAP_405_METHOD_NOT_ALLOWED;

    case LWM2M_CLIENT_ENDPOINT_ID:
        if (instance->client) {
            lwm2m_data_encode_string(instance->ep, data);
            return COAP_205_CONTENT;
        }
        return COAP_404_NOT_FOUND;

    default:
        return COAP_404_NOT_FOUND;
    }
}

static uint8_t _set_int_value(lwm2m_data_t *data, uint32_t *dst)
{
    uint8_t result;
    int64_t value;

    if (lwm2m_data_decode_int(data, &value) == 1) {
        if (value >= 0 && value <= 0xFFFFFFFF) {
            *dst = value;
            result = COAP_204_CHANGED;
        }
        else {
            result = COAP_406_NOT_ACCEPTABLE;
        }
    }
    else {
        result = COAP_400_BAD_REQUEST;
    }
    return result;
}

static uint8_t _write_cb(uint16_t instance_id, int num_data, lwm2m_data_t * data_array,
                         lwm2m_object_t * object)
{
    int i;
    uint8_t result = COAP_404_NOT_FOUND;
    lwm2m_obj_server_inst_t *instance;

    instance = (lwm2m_obj_server_inst_t *)LWM2M_LIST_FIND(object->instanceList, instance_id);
    if (!instance) {
        DEBUG("[lwm2m:server:write] could not find instance %d\n", instance_id);
        return COAP_404_NOT_FOUND;
    }

    i = 0;
    do {
        switch (data_array[i].id) {
        case LWM2M_SERVER_SHORT_ID_ID:
            {
                uint32_t value = instance->short_id;
                result = _set_int_value(&data_array[i], &value);
                if (COAP_204_CHANGED == result) {
                    if (0 < value && 0xFFFF >= value) {
                        instance->short_id = value;
                    }
                    else {
                        DEBUG("[lwm2m:server:write] invalid short ID\n");
                        result = COAP_406_NOT_ACCEPTABLE;
                    }
                }
            }
            break;

        case LWM2M_SERVER_LIFETIME_ID:
            result = _set_int_value(&data_array[i], &instance->lifetime);
            break;

        case LWM2M_SERVER_MIN_PERIOD_ID:
            result = _set_int_value(&data_array[i], &instance->min_period);
            break;

        case LWM2M_SERVER_MAX_PERIOD_ID:
            result = _set_int_value(&data_array[i], &instance->max_period);
            break;

        case LWM2M_SERVER_DISABLE_ID:
            result = COAP_405_METHOD_NOT_ALLOWED;
            break;

        case LWM2M_SERVER_TIMEOUT_ID:
            result = _set_int_value(&data_array[i], &instance->disable_timeout);
            break;

        case LWM2M_SERVER_STORING_ID:
        {
            bool value;
            if (1 == lwm2m_data_decode_bool(&data_array[i], &value)) {
                instance->store = value;
                result = COAP_204_CHANGED;
            }
            else {
                DEBUG("[lwm2m:server:write] can't decode store bool value\n");
                result = COAP_400_BAD_REQUEST;
            }
        }
        break;

        case LWM2M_SERVER_BINDING_ID:
            if (data_array[i].type == LWM2M_TYPE_STRING || data_array[i].type == LWM2M_TYPE_OPAQUE) {
                result = COAP_400_BAD_REQUEST;
                for (unsigned binding = 0; binding < BINDINGS_NUMOF; binding++) {
                    size_t len = strlen(_bindings[binding]);
                    if (data_array[i].value.asBuffer.length == len &&
                        !strncmp((char *)data_array[i].value.asBuffer.buffer, _bindings[binding],
                                 data_array[i].value.asBuffer.length)) {
                        instance->binding = binding;
                        result = COAP_204_CHANGED;
                        break;
                    }
                }
            }
            else {
                result = COAP_400_BAD_REQUEST;
            }
            break;

        case LWM2M_SERVER_UPDATE_ID:
            result = COAP_405_METHOD_NOT_ALLOWED;
            break;

        case LWM2M_CLIENT_ENDPOINT_ID:
            if (!instance->client || data_array[i].type != LWM2M_TYPE_STRING ||
                data_array[i].value.asBuffer.length > MAX_ENDPOINT_NAME) {
                result = COAP_400_BAD_REQUEST;
                break;
            }

            memset(instance->ep, 0, MAX_ENDPOINT_NAME);
            memcpy(instance->ep, data_array[i].value.asBuffer.buffer,
                   data_array[i].value.asBuffer.length);
            result = COAP_204_CHANGED;
            break;

        default:
            return COAP_404_NOT_FOUND;
        }
        i++;
    } while (i < num_data && result == COAP_204_CHANGED);

    return result;
}

static uint8_t _read_cb(uint16_t instance_id, int *num_data, lwm2m_data_t **data_array,
                       lwm2m_object_t *object)
{
    lwm2m_obj_server_inst_t *instance;
    uint8_t result;
    int i;

    instance = (lwm2m_obj_server_inst_t *)lwm2m_list_find(object->instanceList, instance_id);
    if (!instance) {
        DEBUG("[lwm2m:server:read] could not find instance %d\n", instance_id);
        return COAP_404_NOT_FOUND;
    }

    /* when num_data is 0, we need to allocate the data structures and return all resources */
    if (*num_data == 0) {
        uint16_t res_list[] = {
            LWM2M_SERVER_SHORT_ID_ID,
            LWM2M_SERVER_LIFETIME_ID,
            LWM2M_SERVER_MIN_PERIOD_ID,
            LWM2M_SERVER_MAX_PERIOD_ID,
            LWM2M_SERVER_TIMEOUT_ID,
            LWM2M_SERVER_STORING_ID,
            LWM2M_SERVER_BINDING_ID,
            LWM2M_CLIENT_ENDPOINT_ID,
        };
        int res_num;
        
        if (instance->client) {
            res_num = sizeof(res_list)/sizeof(uint16_t);
        }
        else {
            res_num = sizeof(res_list)/sizeof(uint16_t) - 1;
        }

        /* allocate data structs */
        *data_array = lwm2m_data_new(res_num);
        if (*data_array == NULL) {
            DEBUG("[lwm2m:server:read] could not allocate data structs\n");
            return COAP_500_INTERNAL_SERVER_ERROR;
        }

        /* initialize structs with resource IDs */
        *num_data = res_num;
        for (i = 0 ; i < res_num ; i++) {
            (*data_array)[i].id = res_list[i];
        }
    }

    /* get the value for each requested resource */
    i = 0;
    do {
        result = _get_resource_value(&(*data_array)[i], instance);
        i++;
    } while (i < *num_data && result == COAP_205_CONTENT);

    return result;
}

static uint8_t _execute_cb(uint16_t instance_id, uint16_t resource_id, uint8_t *buffer, int length,
                           lwm2m_object_t *object)

{
    (void)buffer;
    (void)length;
 
    lwm2m_obj_server_inst_t *instance = NULL;

    instance = (lwm2m_obj_server_inst_t *)LWM2M_LIST_FIND(object->instanceList, instance_id);
    if (!instance) {
        DEBUG("[lwm2m:server:execute] could not find instance %d\n", instance_id);
        return COAP_404_NOT_FOUND;
    }

    switch (resource_id) {
    case LWM2M_SERVER_DISABLE_ID:
        // TODO: Check this?? -> executed in core, if COAP_204_CHANGED is returned 
        if (instance->disable_timeout > 0) {
            return COAP_204_CHANGED;
        }
        else {
            return COAP_405_METHOD_NOT_ALLOWED;
        }
    case LWM2M_SERVER_UPDATE_ID:
        // TODO: Check this?? -> executed in core, if COAP_204_CHANGED is returned
        return COAP_204_CHANGED;
    default:
        return COAP_405_METHOD_NOT_ALLOWED;
    }
}

static uint8_t _delete_cb(uint16_t instance_id, lwm2m_object_t *object)
{
    lwm2m_obj_server_inst_t *instance = NULL;

    object->instanceList = LWM2M_LIST_RM(object->instanceList, instance_id, &instance);
    if (!instance) {
        DEBUG("[lwm2m:server:delete] could not find instance %d\n", instance_id);
        return COAP_404_NOT_FOUND;
    }

    _free_instance(instance);
    return COAP_202_DELETED;
}

static uint8_t _create_cb(uint16_t instance_id, int num_data, lwm2m_data_t *data_array,
                          lwm2m_object_t * object)
{
    lwm2m_obj_server_inst_t *instance = NULL;
    uint8_t result;

    /* try to allocate an instance */
    instance = _get_free_instance();
    if (!instance) {
        DEBUG("[lwm2m:server:create] could not allocate instance\n");
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    instance->list.id = instance_id;
    object->instanceList = LWM2M_LIST_ADD(object->instanceList, instance);

    result = _write_cb(instance_id, num_data, data_array, object);

    if (result != COAP_204_CHANGED) {
        DEBUG("[lwm2m:server:create] could not write resources\n");
        _delete_cb(instance_id, object);
    }
    else {
        result = COAP_201_CREATED;
    }

    return result;
}


static uint8_t _discover_cb(uint16_t instance_id, int *num_data, lwm2m_data_t **data_array,
                            lwm2m_object_t *object)
{
    uint8_t result;

    (void)instance_id;
    (void)object;

    result = COAP_205_CONTENT;

    // is the server asking for the full object ?
    if (!*num_data) {
        uint16_t res_list[] = {
            LWM2M_SERVER_SHORT_ID_ID,
            LWM2M_SERVER_LIFETIME_ID,
            LWM2M_SERVER_MIN_PERIOD_ID,
            LWM2M_SERVER_MAX_PERIOD_ID,
            LWM2M_SERVER_DISABLE_ID,
            LWM2M_SERVER_TIMEOUT_ID,
            LWM2M_SERVER_STORING_ID,
            LWM2M_SERVER_BINDING_ID,
            LWM2M_SERVER_UPDATE_ID
        };
        int res_num = ARRAY_SIZE(res_list);

        *data_array = lwm2m_data_new(res_num);
        if (*data_array == NULL) {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }

        *num_data = res_num;
        for (int i = 0; i < res_num; i++) {
            (*data_array)[i].id = res_list[i];
        }
    }
    else {
        for (int i = 0; i < *num_data && result == COAP_205_CONTENT; i++) {
            switch ((*data_array)[i].id) {
            case LWM2M_SERVER_SHORT_ID_ID:
            case LWM2M_SERVER_LIFETIME_ID:
            case LWM2M_SERVER_MIN_PERIOD_ID:
            case LWM2M_SERVER_MAX_PERIOD_ID:
            case LWM2M_SERVER_DISABLE_ID:
            case LWM2M_SERVER_TIMEOUT_ID:
            case LWM2M_SERVER_STORING_ID:
            case LWM2M_SERVER_BINDING_ID:
            case LWM2M_SERVER_UPDATE_ID:
                break;
            default:
                result = COAP_404_NOT_FOUND;
            }
        }
    }

    return result;
}

lwm2m_object_t *lwm2m_object_server_get(void)
{
    return &_server_object;
}

lwm2m_object_t *lwm2m_object_client_get(void)
{
    return &_client_object;
}

int _instance_create(lwm2m_object_t *object, uint16_t instance_id,
                     const lwm2m_obj_server_args_t *args, bool client)
{
    assert(object);
    assert(args);

    lwm2m_obj_server_inst_t  *instance = NULL;

    instance = _get_free_instance();
    if (!instance) {
        DEBUG("[lwm2m:server]: can't allocate new instance\n");
        return -1;
    }

    if (args->short_id < 1) {
        DEBUG("[lwm2m:server]: invalid short server ID %d\n", args->short_id);
        _free_instance(instance);
        return -1;
    }

    instance->list.id = instance_id;
    instance->short_id = args->short_id;
    instance->lifetime = args->lifetime;
    instance->store = args->notification_storing;
    instance->min_period = args->min_period;
    instance->max_period = args->max_period;
    instance->disable_timeout = args->disable_timeout;
    instance->binding = args->binding;
    instance->client = client;

    if (client) {
        lwm2m_obj_client_args_t *_args = (lwm2m_obj_client_args_t*)args;
        if (_args->endpoint) {
            strcpy(instance->ep, _args->endpoint);
        }
    }

    /* add the new instance to the list */
    object->instanceList = LWM2M_LIST_ADD(object->instanceList, instance);
    DEBUG("[lwm2m:server]: added server instance with ID %d\n", instance_id);
    return 0;
}

int lwm2m_object_server_instance_create(lwm2m_object_t *object, uint16_t instance_id,
                                        const lwm2m_obj_server_args_t *args)
{
    assert(object);
    assert(args);

    if (object->objID != LWM2M_SERVER_OBJECT_ID) {
        return -1;
    }
    return _instance_create(object, instance_id, args, false);
}

int lwm2m_object_client_instance_create(lwm2m_object_t *object, uint16_t instance_id,
                                        const lwm2m_obj_client_args_t *args)
{
    assert(object);
    assert(args);

    if (object->objID != LWM2M_CLIENT_OBJECT_ID) {
        return -1;
    }
    return _instance_create(object, instance_id, (lwm2m_obj_server_args_t *)args, true);
}
