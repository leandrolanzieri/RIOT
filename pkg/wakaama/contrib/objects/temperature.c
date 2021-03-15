/*
 * Copyright (C) 2021 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
/**
 * @{
 * @ingroup     lwm2m_objects_temperature
 *
 * @file
 * @brief       IPSO Temperature object implementation for LwM2M client using Wakaama
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include "liblwm2m.h"
#include "lwm2m_client.h"
#include "objects/common.h"
#include "objects/temperature.h"

#define ENABLE_DEBUG 1
#include "debug.h"

typedef struct lwm2m_obj_temperature_inst {
    lwm2m_list_t list;
    double last_value;
    double min_value;
    double max_value;
    double min_range;
    double max_range;
    bool reset;
    char *units;
    size_t units_len;
} lwm2m_obj_temperature_inst_t;

static uint8_t _read_cb(uint16_t instance_id, int *num_data, lwm2m_data_t **data_array,
                        lwm2m_object_t *object);

static lwm2m_object_t _temperature_object = {
    .next           = NULL,
    .objID          = LWM2M_TEMPERATURE_OBJECT_ID,
    .instanceList   = NULL,
    .readFunc       = _read_cb,
    .writeFunc      = NULL,
    .executeFunc    = NULL,
    .createFunc     = NULL,
    .deleteFunc     = NULL,
    .discoverFunc   = NULL,
    .userData       = NULL
};

static uint8_t _get_value(lwm2m_data_t *data, lwm2m_obj_temperature_inst_t *instance)
{
    assert(data);
    assert(instance);

    switch (data->id) {
    case LWM2M_TEMPERATURE_VALUE_ID:
        DEBUG("[lwm2m:temperature] encoding value\n");
        lwm2m_data_encode_float(instance->last_value, data);
        DEBUG("[lwm2m:temperature] encoded\n");
        break;

    case LWM2M_TEMPERATURE_MIN_MEAS_ID:
        lwm2m_data_encode_double(instance->min_value, data);
        break;

    case LWM2M_TEMPERATURE_MAX_MEAS_ID:
        lwm2m_data_encode_double(instance->max_value, data);
        break;

    case LWM2M_TEMPERATURE_MIN_RANGE_ID:
        lwm2m_data_encode_double(instance->min_range, data);
        break;

    case LWM2M_TEMPERATURE_MAX_RANGE_ID:
        lwm2m_data_encode_double(instance->max_range, data);
        break;

    case LWM2M_TEMPERATURE_UNITS_ID:
        lwm2m_data_encode_nstring(instance->units, instance->units_len, data);
        break;

    default:
        return COAP_404_NOT_FOUND;
    }
    return COAP_205_CONTENT;
}

static uint8_t _read_cb(uint16_t instance_id, int *num_data, lwm2m_data_t **data_array,
                        lwm2m_object_t *object)
{
    lwm2m_obj_temperature_inst_t *instance;
    uint8_t result;
    int i = 0;

    /* try to get the requested instance from the object list */
    instance = (lwm2m_obj_temperature_inst_t *)lwm2m_list_find(object->instanceList, instance_id);
    if (!instance) {
        DEBUG("[lwm2m:temperature:read]: can't find instance %d\n", instance_id);
        result = COAP_404_NOT_FOUND;
        goto out;
    }

    /* if the number of resources is not specified, we need to read all resources */
    if (!*num_data) {
        DEBUG("[lwm2m:temperature:read]: reading all resources\n");

        uint16_t res_list[] = {
            LWM2M_TEMPERATURE_VALUE_ID,
            LWM2M_TEMPERATURE_MIN_MEAS_ID,
            LWM2M_TEMPERATURE_MAX_MEAS_ID,
            LWM2M_TEMPERATURE_MIN_RANGE_ID,
            LWM2M_TEMPERATURE_MAX_RANGE_ID,
            LWM2M_TEMPERATURE_UNITS_ID
        };

        /* allocate structures to return resources */
        int res_num = ARRAY_SIZE(res_list);
        *data_array = lwm2m_data_new(res_num);

        if (NULL == data_array) {
            result = COAP_500_INTERNAL_SERVER_ERROR;
            goto out;
        }

        /* return the number of resources being read */
        *num_data = res_num;

        /* set the IDs of the resources in the data structures */
        for (i = 0; i < res_num; i++) {
            (*data_array)[i].id = res_list[i];
        }
    }

    /* now get the values */
    i = 0;
    do {
        DEBUG("[lwm2m:temperature:read]: reading resource %d\n", (*data_array)[i].id);
        result = _get_value(&(*data_array)[i], instance);
        i++;
    } while (i < *num_data && COAP_205_CONTENT == result);

out:
    return result;
}

int lwm2m_object_temperature_value_update(lwm2m_client_data_t *client_data, uint16_t instance_id,
                                          double val)
{
    assert(client_data);

    lwm2m_object_t *object = lwm2m_get_object_by_id(client_data, LWM2M_TEMPERATURE_OBJECT_ID);
    if (!object) {
        DEBUG("[lwm2m:temperature]: can't find object. Make sure the client is initialized\n");
        return -1;
    }

    lwm2m_obj_temperature_inst_t *instance;
    instance = (lwm2m_obj_temperature_inst_t *)lwm2m_list_find(object->instanceList, instance_id);
    if (!instance) {
        DEBUG("[lwm2m:temperature]: can't find instance %d\n", instance_id);
        return -1;
    }

    /* update values and notify the engine in case there are observers registered */
    DEBUG("[lwm2m:temperature]: value updated\n");
    instance->last_value = val;
    lwm2m_uri_t uri = {
        .objectId = LWM2M_TEMPERATURE_OBJECT_ID,
        .instanceId = instance_id,
        .resourceId = LWM2M_TEMPERATURE_VALUE_ID,
        .flag = LWM2M_URI_FLAG_OBJECT_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_RESOURCE_ID
    };

    lwm2m_resource_value_changed(client_data->lwm2m_ctx, &uri);

    if (instance->reset || instance->min_value > val) {
        DEBUG("[lwm2m:temperature]: min value updated\n");
        instance->min_value = val;
        uri.resourceId = LWM2M_TEMPERATURE_MIN_MEAS_ID;
        lwm2m_resource_value_changed(client_data->lwm2m_ctx, &uri);
    }

    if (instance->reset || instance->max_value < val) {
        DEBUG("[lwm2m:temperature]: max updated\n");
        instance->max_value = val;
        uri.resourceId = LWM2M_TEMPERATURE_MAX_MEAS_ID;
        lwm2m_resource_value_changed(client_data->lwm2m_ctx, &uri);
    }

    instance->reset = false;

    return 0;
}

lwm2m_object_t *lwm2m_object_temperature_get(void)
{
    return &_temperature_object;
}

int lwm2m_object_temperature_instance_create(lwm2m_object_t *object, uint16_t instance_id,
                                             double min_range, double max_range, const char *units,
                                             size_t units_len)
{
    assert(object);

    lwm2m_obj_temperature_inst_t *instance = NULL;

    if (object->objID != LWM2M_TEMPERATURE_OBJECT_ID) {
        goto out;
    }

    /* try to allocate an instance */
    instance = (lwm2m_obj_temperature_inst_t *)lwm2m_malloc(sizeof(lwm2m_obj_temperature_inst_t));
    if (!instance) {
        DEBUG("[lwm2m:temperature]: can't allocate new instance\n");
        goto out;
    }

    memset(instance, 0, sizeof(lwm2m_obj_temperature_inst_t));

    instance->list.id = instance_id;
    instance->min_value = min_range;
    instance->max_value = min_range;
    instance->last_value = min_range;
    instance->min_range = min_range;
    instance->max_range = max_range;
    instance->reset = false;

    /* if units are specified, copy locally */
    if (units) {
        instance->units = lwm2m_malloc(units_len);
        if (instance->units) {
            instance->units_len = units_len;
            memcpy(instance->units, units, units_len);
        }
        else {
            DEBUG("[lwm2m:temperature]: can't allocate units string\n");
            goto free_out;
        }
    }

    DEBUG("[lwm2m:temperature]: new instance with ID %d\n", instance_id);

    /* add the new instance to the list */
    object->instanceList = LWM2M_LIST_ADD(object->instanceList, instance);
    goto out;

free_out:
    lwm2m_free(instance);
out:
    return instance == NULL ? -1 : 0;
}
