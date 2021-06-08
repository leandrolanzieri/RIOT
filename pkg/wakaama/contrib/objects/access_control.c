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
 * @ingroup     lwm2m_objects_access_control
 *
 * @file
 * @brief       Access Control object implementation for LwM2M client using Wakaama
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include <string.h>
#include "assert.h"

#include "liblwm2m.h"

#include "objects/access_control.h"

#define ENABLE_DEBUG    0
#include "debug.h"

/**
 * @brief Internal value to mark an ACL from the pool as used.
 */
#define _ACL_USED           (0x80)

/**
 * @brief Mask of access flags for an ACL.
 */
#define _ACL_FLAGS_MASK     (0x1F)

/**
 * @brief Access Control List (ACL) instance.
 */
typedef struct lwm2m_obj_acc_ctrl_acl {
    lwm2m_list_t list;      /**< list handle */
    uint16_t access;        /**< access flags */
} lwm2m_obj_acc_ctrl_acl_t;

/**
 * @brief Access Control object instance.
 */
typedef struct lwm2m_obj_acc_ctrl_inst {
    lwm2m_list_t list;                          /**< list handle */
    uint16_t obj_id;                            /**< object id */
    uint16_t obj_inst_id;                       /**< object instance id */
    uint16_t owner;                             /**< access control owner */
    lwm2m_obj_acc_ctrl_acl_t *acl_list;         /**< access control lists */
    bool client;                                /**< is an instance of Client Access Control */
} lwm2m_obj_acc_ctrl_inst_t;

/**
 * @brief   'Read' callback for the Access Control object.
 */
static uint8_t _read_cb(uint16_t instance_id, int *num_data, lwm2m_data_t **data_array,
                        lwm2m_object_t *object);

/**
 * @brief   'Write' callback for the Access Control object.
 */
static uint8_t _write_cb(uint16_t instance_id, int num_data, lwm2m_data_t *data_array,
                         lwm2m_object_t *object);

/**
 * @brief   'Delete' callback for the Access Control object.
 */
static uint8_t _delete_cb(uint16_t instance_id, lwm2m_object_t *object);

/**
 * @brief   'Create' callback for the Access Control object.
 */
static uint8_t _create_cb(uint16_t instance_id, int num_data, lwm2m_data_t *data_array,
                          lwm2m_object_t *object);

/**
 * @brief   Try to allocate an access control object instance from the pool.
 * @retval  Pointer to the instance on success
 * @retval  NULL otherwise
 */
static lwm2m_obj_acc_ctrl_inst_t *_get_free_instance(void);

/**
 * @brief   Free an instance allocated by @ref _get_free_instance.
 * @param[in] instance      Pointer to instance to free.
 */
static void _free_instance(lwm2m_obj_acc_ctrl_inst_t *instance);

/**
 * @brief   Try to allocate an ACL from the pool.
 * @retval Pointer to instance on success
 * @retval NULL otherwise
 */
static lwm2m_obj_acc_ctrl_acl_t *_get_free_acl(void);

/**
 * @brief   Free an ACL allocated by @ref _get_free_acl.
 * @param[in] acl_list      Pointer to ACL to free.
 */
static void _free_acl_list(lwm2m_obj_acc_ctrl_acl_t *acl_list);

/**
 * @brief Pool of Access Control object instances.
 */
static lwm2m_obj_acc_ctrl_inst_t _instances[CONFIG_LWM2M_ACCESS_CONTROL_INSTANCES_MAX] = { 0 };

/**
 * @brief Pool of ACLs.
 */
static lwm2m_obj_acc_ctrl_acl_t _acls[CONFIG_LWM2M_ACCESS_CONTROL_ACLS_MAX] = { 0 };

/**
 * @brief Access Control object implementation descriptor.
 */
static lwm2m_object_t _acc_control_object = {
    .next           = NULL,
    .objID          = LWM2M_ACCESS_CONTROL_OBJECT_ID,
    .instanceList   = NULL,
    .readFunc       = _read_cb,
    .writeFunc      = _write_cb,
    .createFunc     = _create_cb,
    .deleteFunc     = _delete_cb,
    .executeFunc    = NULL,
    .discoverFunc   = NULL,
    .userData       = NULL
};

/**
 * @brief Client Access Control object implementation descriptor.
 */
static lwm2m_object_t _client_acc_control_object = {
    .next           = NULL,
    .objID          = LWM2M_CLIENT_ACCESS_CONTROL_OBJECT_ID,
    .instanceList   = NULL,
    .readFunc       = _read_cb,
    .writeFunc      = _write_cb,
    .createFunc     = _create_cb,
    .deleteFunc     = _delete_cb,
    .executeFunc    = NULL,
    .discoverFunc   = NULL,
    .userData       = NULL
};

static lwm2m_obj_acc_ctrl_inst_t *_get_free_instance(void)
{
    lwm2m_obj_acc_ctrl_inst_t * instance = NULL;

    for (unsigned i = 0; i < CONFIG_LWM2M_ACCESS_CONTROL_INSTANCES_MAX; i++) {
        if (!_instances[i].owner) {
            DEBUG("[lwm2m:access_control] instance %d is free\n", i);
            instance = &_instances[i];
            break;
        }
    }
    memset(instance, 0, sizeof(lwm2m_obj_acc_ctrl_inst_t));

    return instance;
}

static void _free_instance(lwm2m_obj_acc_ctrl_inst_t *instance)
{
    if (!instance) {
        return;
    }

    _free_acl_list(instance->acl_list);
    memset(instance, 0, sizeof(lwm2m_obj_acc_ctrl_inst_t));
}

static lwm2m_obj_acc_ctrl_acl_t *_get_free_acl(void)
{
    lwm2m_obj_acc_ctrl_acl_t *acl = NULL;

    for (unsigned i = 0; i < CONFIG_LWM2M_ACCESS_CONTROL_ACLS_MAX; i++) {
        if (!(_acls[i].access & _ACL_USED)) {
            acl = &_acls[i];
            break;
        }
    }
    memset(acl, 0, sizeof(lwm2m_obj_acc_ctrl_acl_t));

    return acl;
}

static void _free_acl_list(lwm2m_obj_acc_ctrl_acl_t *acl_list)
{
    while (acl_list) {
        lwm2m_obj_acc_ctrl_acl_t * next = (lwm2m_obj_acc_ctrl_acl_t *)acl_list->list.next;
        memset(acl_list, 0, sizeof(lwm2m_obj_acc_ctrl_acl_t));
        acl_list = next;
    }
}

static uint8_t _get_value(lwm2m_data_t *data, lwm2m_obj_acc_ctrl_inst_t *instance)
{
    assert(data);
    assert(instance);

    switch (data->id) {
    case LWM2M_ACCESS_CONTROL_OBJ_ID_ID:
        lwm2m_data_encode_int(instance->obj_id, data);
        return COAP_205_CONTENT;

    case LWM2M_ACCESS_CONTROL_OBJ_INST_ID_ID:
        lwm2m_data_encode_int(instance->obj_inst_id, data);
        return COAP_205_CONTENT;

    case LWM2M_ACCESS_CONTROL_AC_OWNER:
        lwm2m_data_encode_int(instance->owner, data);
        return COAP_205_CONTENT;

    case LWM2M_ACCESS_CONTROL_ACL_ID:
    {
        /* count the number of ACLs in this instance to allocate the needed data structs */
        unsigned acl_num = 0;
        lwm2m_obj_acc_ctrl_acl_t *acl = instance->acl_list;
        while (acl) {
            acl_num++;
            acl = (lwm2m_obj_acc_ctrl_acl_t *)acl->list.next;
        }

        if (!acl_num) {
            DEBUG("[lwm2m:access_control:get_val]: no ACLs for %d\n", instance->list.id);
            data->value.asChildren.array = NULL;
            data->value.asChildren.count = 0;
            data->type = LWM2M_TYPE_MULTIPLE_RESOURCE;
            return COAP_205_CONTENT;
        }

        /* try to allocate data structs */
        lwm2m_data_t *data_array = lwm2m_data_new(acl_num);
        if (!data_array) {
            DEBUG("[lwm2m:access_control:get_val]: could not allocate data structs\n");
            return COAP_500_INTERNAL_SERVER_ERROR;
        }

        /* go over the list again, but this time copy the values */
        acl_num = 0;
        acl = instance->acl_list;

        while (acl) {
            data_array[acl_num].id = acl->list.id;
            lwm2m_data_encode_int(acl->access & _ACL_FLAGS_MASK, &data_array[acl_num]);

            acl_num++;
            acl = (lwm2m_obj_acc_ctrl_acl_t *)acl->list.next;
        }

        /* this encodes the array of data into the provided data structure */
        lwm2m_data_encode_instances(data_array, acl_num, data);
        return COAP_205_CONTENT;
    }

    default:
        return COAP_404_NOT_FOUND;
    }
}

static uint8_t _read_cb(uint16_t instance_id, int *num_data, lwm2m_data_t **data_array,
                        lwm2m_object_t *object)
{
    lwm2m_obj_acc_ctrl_inst_t *instance;
    uint8_t result = COAP_404_NOT_FOUND;
    int i = 0;

    /* try to get the requested instance from the object list */
    instance = (lwm2m_obj_acc_ctrl_inst_t *)lwm2m_list_find(object->instanceList, instance_id);
    if (!instance) {
        DEBUG("[lwm2m:access_control:read]: can't find instance %d\n", instance_id);
        return COAP_404_NOT_FOUND;
    }

    /* if the number of resources is not specified, we need to read all resources */
    if (!*num_data) {
        DEBUG("[lwm2m:light_control:read]: reading all resources\n");

        uint16_t res_list[] = {
            LWM2M_ACCESS_CONTROL_OBJ_ID_ID,
            LWM2M_ACCESS_CONTROL_OBJ_INST_ID_ID,
            LWM2M_ACCESS_CONTROL_AC_OWNER,
            LWM2M_ACCESS_CONTROL_ACL_ID
        };

        /* allocate structures to return resources */
        int res_num = ARRAY_SIZE(res_list);
        *data_array = lwm2m_data_new(res_num);

        if (NULL == *data_array) {
            return COAP_500_INTERNAL_SERVER_ERROR;
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
        DEBUG("[lwm2m:access_control:read]: reading resource %d\n", (*data_array)[i].id);
        result = _get_value(&(*data_array)[i], instance);
        i++;
    } while (i < *num_data && COAP_205_CONTENT == result);

    return result;
}

static uint8_t _write_integer(uint16_t *out, lwm2m_data_t *data, uint16_t min, uint16_t max)
{
    assert(out);
    assert(data);

    int64_t value;

    if (lwm2m_data_decode_int(data, &value) != 1) {
        DEBUG("[lwm2m:access_control:write]: can't decode integer\n");
        return COAP_400_BAD_REQUEST;
    }

    if (value < min || value >= max) {
        DEBUG("[lwm2m:access_control:write]: invalid integer\n");
        return COAP_406_NOT_ACCEPTABLE;
    }

    *out = value;
    return COAP_204_CHANGED;
}

static uint8_t _write_resources(uint16_t instance_id, int num_data, lwm2m_data_t *data_array,
                                lwm2m_object_t *object, bool creating)
{
    uint8_t result = COAP_204_CHANGED;
    int64_t value;
    lwm2m_obj_acc_ctrl_inst_t *instance = NULL;

    /* try to get the requested instance from the object list */
    instance = (lwm2m_obj_acc_ctrl_inst_t *)lwm2m_list_find(object->instanceList, instance_id);
    if (!instance) {
        DEBUG("[lwm2m:access_control:write]: can't find instance %d\n", instance_id);
        return COAP_404_NOT_FOUND;
    }

    for (int i = 0; i < num_data && result == COAP_204_CHANGED; i++) {
        switch (data_array[i].id) {
        case LWM2M_ACCESS_CONTROL_OBJ_ID_ID:
            /* only write resource if creating instance */
            if (!creating) {
                DEBUG("[lwm2m:access_control:write]: can't write Obj ID\n");
                result = COAP_405_METHOD_NOT_ALLOWED;
                break;
            }

            result = _write_integer(&instance->obj_id, &data_array[i], 1, 65534);
            break;
        
        case LWM2M_ACCESS_CONTROL_OBJ_INST_ID_ID:
            /* only write resource if creating instance */
            if (!creating) {
                DEBUG("[lwm2m:access_control:write]: can't write Obj inst ID\n");
                result = COAP_405_METHOD_NOT_ALLOWED;
                break;
            }

            result = _write_integer(&instance->obj_inst_id, &data_array[i], 0, 65535);
            break;

        case LWM2M_ACCESS_CONTROL_AC_OWNER:
            result = _write_integer(&instance->owner, &data_array[i], 0, 65535);
            break;

        case LWM2M_ACCESS_CONTROL_ACL_ID:
        {
            if (data_array[i].value.asChildren.count != 0 && !data_array[i].value.asChildren.array) {
                DEBUG("[lwm2m:access_control:write]: malformed ACL list\n");
                result = COAP_400_BAD_REQUEST;
                break;
            }

            /* Wakaama v1.0 has no indication for partial update of Multiple-Instance resources, so
             * the array of resources is replaced (see 'Replace' mechanism in LwM2M section 5.4.3).
             */

            /* save current list */
            lwm2m_obj_acc_ctrl_acl_t *acl_old = instance->acl_list;
            instance->acl_list = NULL;

            /* check if only emptying array */
            if (!data_array[i].value.asChildren.count) {
                DEBUG("[lwm2m:access_control:write]: emptying ACL list\n");
                _free_acl_list(acl_old);
                result = COAP_204_CHANGED;
                break;
            }

            /* data_list will hold an array of integers, the value will be the access, the id will
             * be the instance ID for the ACL */
            lwm2m_data_t *data_list = data_array[i].value.asChildren.array;
            for (unsigned i = 0; i < data_array[i].value.asChildren.count; i++) {
                if (lwm2m_data_decode_int(&data_list[i], &value) != 1) {
                    DEBUG("[lwm2m:access_control:write]: can't decode ACL access\n");
                    result = COAP_400_BAD_REQUEST;
                    break;
                }

                if (value < 0 || value > _ACL_FLAGS_MASK) {
                    DEBUG("[lwm2m:access_control:write]: invalid access flags\n");
                    result = COAP_406_NOT_ACCEPTABLE;
                    break;
                }

                lwm2m_obj_access_control_acl_args_t args = {
                    .res_inst_id = data_list[i].id,
                    .access = (uint16_t)value
                };

                if (lwm2m_object_access_control_add(object, instance_id, &args) != 0) {
                    DEBUG("[lwm2m:access_control:write]: can't add ACL instance\n");
                    result = COAP_500_INTERNAL_SERVER_ERROR;
                    break;
                }

                result = COAP_204_CHANGED;
            }

            /* if no errors, free old list */
            if (result == COAP_204_CHANGED) {
                _free_acl_list(acl_old);
            }
            else {
                /* something went wrong, empty the list so far and restore old values */
                _free_acl_list(instance->acl_list);
                instance->acl_list = acl_old;
            }
            break;
        }
        default:
            result = COAP_404_NOT_FOUND;
        }
    }

    return result;
}

static uint8_t _delete_cb(uint16_t instance_id, lwm2m_object_t *object)
{
    lwm2m_obj_acc_ctrl_inst_t *instance = NULL;
    object->instanceList = LWM2M_LIST_RM(object->instanceList, instance_id, &instance);

    if (!instance) {
        DEBUG("[lwm2m:access_control:delete]: can't find instance with ID %d\n", instance_id);
        return COAP_404_NOT_FOUND;
    }

    _free_instance(instance);

    DEBUG("[lwm2m:access_control]: deleted instance with ID %d\n", instance_id);
    return COAP_202_DELETED;
}

static uint8_t _write_cb(uint16_t instance_id, int num_data, lwm2m_data_t *data_array,
                         lwm2m_object_t *object)
{
    return _write_resources(instance_id, num_data, data_array, object, false);
}

static uint8_t _create_cb(uint16_t instance_id, int num_data, lwm2m_data_t *data_array,
                          lwm2m_object_t *object)
{

    lwm2m_obj_acc_ctrl_inst_t *instance = _get_free_instance();
    if (!instance) {
        DEBUG("[lwm2m:access_control:create]: can't allocate new instance\n");
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    /* add new instance to object's instance list */
    instance->list.id = instance_id;
    instance->list.next = NULL;
    instance->acl_list = NULL;
    object->instanceList = LWM2M_LIST_ADD(object->instanceList, instance);

    /* write the provided resources */
    uint8_t res = _write_resources(instance_id, num_data, data_array, object, true);
    if (res != COAP_204_CHANGED) {
        /* something went wrong, delete the instance */
        DEBUG("[lwm2m:access_control:create]: could not write resources to new instance\n");
        _delete_cb(instance_id, object);
        return res;
    }

    DEBUG("[lwm2m:access_control:create]: created new instance with ID %d\n", instance_id);
    return COAP_201_CREATED;
}

lwm2m_object_t *lwm2m_object_access_control_get(void)
{
    return &_acc_control_object;
}

lwm2m_object_t *lwm2m_object_client_access_control_get(void)
{
    return &_client_acc_control_object;
}

int lwm2m_object_access_control_instance_create(lwm2m_object_t *object, uint16_t instance_id,
                                                const lwm2m_obj_access_control_args_t *args)
{
    assert(object);
    assert(args);

    lwm2m_obj_acc_ctrl_inst_t *instance = NULL;

    if (object->objID != LWM2M_ACCESS_CONTROL_OBJECT_ID) {
        return -1;
    }

    instance = _get_free_instance();

    if (!instance) {
        DEBUG("[lwm2m:access_control]: can't allocate new instance\n");
        return -1;
    }

    if (args->obj_id < 1 || args->obj_id > 65534) {
        DEBUG("[lwm2m:access_control]: invalid object ID %d\n", args->obj_id);
        _free_instance(instance);
        return -1;
    }

    instance->list.id = instance_id;
    instance->obj_id = args->obj_id;
    instance->obj_inst_id = args->obj_inst_id;
    instance->owner = args->owner;
    instance->acl_list = NULL;

    /* add the new instance to the list */
    object->instanceList = LWM2M_LIST_ADD(object->instanceList, instance);
    return 0;
}

int lwm2m_object_access_control_add(lwm2m_object_t *object, uint16_t instance_id,
                                    const lwm2m_obj_access_control_acl_args_t *args)
{
    assert(object);
    assert(args);

    lwm2m_obj_acc_ctrl_inst_t *instance = NULL;

    /* try to get the requested instance from the object list */
    instance = (lwm2m_obj_acc_ctrl_inst_t *)lwm2m_list_find(object->instanceList, instance_id);

    if (!instance) {
        DEBUG("[lwm2m:access_control]: can't find instance %d\n", instance_id);
        return -1;
    }

    lwm2m_obj_acc_ctrl_acl_t *acl = _get_free_acl();

    if (!acl) {
        DEBUG("[lwm2m:access_control]: can't allocate new ACL\n");
        return -1;
    }

    acl->access = _ACL_USED | (args->access & _ACL_FLAGS_MASK);
    acl->list.id = args->res_inst_id;

    /* add the new ACL to the list */
    instance->acl_list = (lwm2m_obj_acc_ctrl_acl_t *)LWM2M_LIST_ADD(instance->acl_list, acl);

    return 0;
}

static lwm2m_obj_acc_ctrl_inst_t * _get_instance(const lwm2m_uri_t *uri,
                                                 const lwm2m_object_t *object)
{

    lwm2m_obj_acc_ctrl_inst_t *instance = (lwm2m_obj_acc_ctrl_inst_t *)object->instanceList;
    while (instance) {
        if (instance->obj_id == uri->objectId && instance->obj_inst_id == uri->instanceId) {
            break;
        }
        instance = (lwm2m_obj_acc_ctrl_inst_t *)instance->list.next;
    }

    if (!instance) {
        DEBUG("[lwm2m:access_control]: no ACL for the given resource\n");
    }

    return instance;
}

int lwm2m_object_access_control_get_access(uint16_t server_id, lwm2m_uri_t *uri,
                                           lwm2m_object_t *object)
{
    int access = 0;
    DEBUG("[lwm2m:access_control]: getting access for server %d\n", server_id);

    if (!object || !uri) {
        return 0;
    }

    lwm2m_obj_acc_ctrl_inst_t *instance = _get_instance(uri, object);
    lwm2m_obj_acc_ctrl_acl_t *acl = NULL;
    if (!instance) {
        return -1; /* no access information available */
    }

    acl = (lwm2m_obj_acc_ctrl_acl_t *)LWM2M_LIST_FIND(instance->acl_list, server_id);
    
    if (!acl) {
        DEBUG("[lwm2m:access_control]: no specific access, use default\n");
        /* try to find the default access for the resource */
        acl = (lwm2m_obj_acc_ctrl_acl_t *)LWM2M_LIST_FIND(instance->acl_list, 0);
    }

    if (acl) {
        access = acl->access & _ACL_FLAGS_MASK;
    }

    return access;
}

int lwm2m_object_access_control_get_owner(const lwm2m_uri_t *uri, const lwm2m_object_t *object)
{
    DEBUG("[lwm2m:access_control]: getting owner server\n");

    if (!object || !uri) {
        return 0;
    }

    const lwm2m_obj_acc_ctrl_inst_t *instance = _get_instance(uri, object);
    if (!instance) {
        return -1; /* no access information available */
    }

    return instance->owner;
}
