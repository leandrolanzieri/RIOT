/*
 * Copyright (C) 2021 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
/**
 * @{
 * @ingroup     lwm2m_objects_security
 *
 * @file
 * @brief       Security object implementation for LwM2M client using Wakaama
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include "liblwm2m.h"

#include "liblwm2m.h"
#include "objects/security.h"
#include "lwm2m_client_config.h"
#include "kernel_defines.h"
#include "net/credman.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ENABLE_DEBUG    1
#include "debug.h"

/**
 * @brief Descriptor of a LwM2M Security object instance (Object ID = 0)
 */
typedef struct lwm2m_obj_security_inst {
    lwm2m_list_t list;              /**< Linked list handle */
    char *uri;                      /**< Server URI */
    bool is_bootstrap;              /**< The associated server is a Bootstrap-Server */
    uint8_t security_mode;          /**< UDP security mode to use with this server (see liblwm2m.h) */
    credman_tag_t cred_tag;         /**< Tag of the credential to use with the LwM2M server */
    void *pub_key_or_id;            /**< Public key or ID */
    size_t pub_key_or_id_len;       /**< Length of public key or ID */
    void *server_pub_key;           /**< Server public key */
    size_t server_pub_key_len;      /**< Length of server public key */
    void *secret_key;               /**< Secret or private key */
    size_t secret_key_len;          /**< Length of secret or private key */
    uint8_t sms_security_mode;      /**< SMS security mode to use with this server */
    char *sms_params;               /**< SMS binding KIc, KID, SPI and TAR */
    uint16_t sms_params_len;        /**< Length of @ref lwm2m_obj_security_inst_t::sms_params */
    char *sms_secret;               /**< SMS binding secret key */
    uint16_t sms_secret_len;        /**< Length of @ref lwm2m_obj_security_inst_t::sms_secret */
    char *sms_server_number;        /**< MSISDN used by the LwM2M to send messages to the server */
    uint16_t short_id;              /**< Short ID to reference to the server */
    uint32_t client_hold_off_time;  /**< Hold off time for registration */
    uint32_t bs_account_timeout;    /**< Timeout for Bootstrap-Server account deletion */
} lwm2m_obj_security_inst_t;

/**
 * @brief 'Read' callback for the security object.
 *
 * @param[in] instance_id       ID of the instance to read
 * @param[in, out] num_data     Number of resources requested. 0 means all.
 * @param[in, out] data_array   Initialized data array to output the values,
 *                              when @p num_data != 0. Uninitialized otherwise.
 * @param[in] object            Security object pointer
 *
 * @return COAP_205_CONTENT                 on success
 * @return COAP_404_NOT_FOUND               when resource can't be found
 * @return COAP_500_INTERNAL_SERVER_ERROR   otherwise
 */
static uint8_t _read_cb(uint16_t instance_id, int *num_data,
                        lwm2m_data_t **data_array,
                        lwm2m_object_t *object);

/**
 * @brief 'Write' callback for the security object.
 *
 * @param[in] instance_id       ID of the instance to write to
 * @param[in] num_data          Number of resources to write
 * @param[in] data_array        Array of data to write
 * @param[in] object            Security object pointer
 *
 * @return COAP_204_CHANGED                 on success
 * @return COAP_400_BAD_REQUEST             otherwise
 */
static uint8_t _write_cb(uint16_t instance_id, int num_data,
                         lwm2m_data_t * data_array,
                         lwm2m_object_t * object);

/**
 * @brief 'Delete' callback for the security object.
 *
 * @param[in] instance_id       ID of the instance to delete
 * @param[in] object            Security object pointer
 *
 * @return COAP_202_DELETED                 on success
 * @return COAP_404_NOT_FOUND               when the instance can't be found
 */
static uint8_t _delete_cb(uint16_t instance_id, lwm2m_object_t *object);

/**
 * @brief 'Create' callback for the security object.
 *
 * @param[in] instance_id       ID of the instance to create
 * @param[in] num_data          Number of resources to write
 * @param[in] data_array        Array of data to write
 * @param[in] object            Security object pointer
 *
 * @return COAP_201_CREATED                 on success
 * @return COAP_500_INTERNAL_SERVER_ERROR   otherwise
 */
static uint8_t _create_cb(uint16_t instance_id, int num_data,
                          lwm2m_data_t * data_array,
                          lwm2m_object_t * object);

/**
 * @brief Get a value from a security object instance.
 *
 * @param data[in, out]     Data structure indicating the id of the resource
 *                          to get the value of. It will contain the value if
 *                          successful.
 * @param instance[in]      Instance to get the data from.
 * @return 0 on success
 * @return <0 otherwise
 */
static int _get_value(lwm2m_data_t *data, lwm2m_obj_security_inst_t *instance);

/**
 * @brief Set a credential-related value to a security object instance.
 *
 * @param[in] data          Data to write.
 * @param[in] instance      Instance to write the data to.
 *
 * @return COAP_204_CHANGED                 on success
 * @return COAP_500_INTERNAL_SERVER_ERROR   otherwise
 */
static int _set_cred_value(lwm2m_data_t *data, lwm2m_obj_security_inst_t *instance);

/**
 * @brief Update a credential in the credman registry with the current instance information.
 *
 * @param[in] instance      Instance to update the credential to.
 */
static void _update_credential(lwm2m_obj_security_inst_t *instance);

/**
 * @brief Free an allocated buffer in @p dst, allocate a new one of length @p len and copy from
 *        @p src.
 *
 * @param[in, out] dst      Buffer to free and to copy to.
 * @param[in]      src      Buffer to copy from.
 * @param[in]      len      Number of bytes to copy from @p src.
 *
 * @return COAP_204_CHANGED on success
 * @return COAP_500_INTERNAL_SERVER_ERROR otherwise
 */
static int _free_and_copy_buffer(char *dst, uint8_t *src, size_t len);

/**
 * @brief Free an allocated buffer @p buf, when it is not NULL.
 *
 * @param[in] buf          Buffer to free.
 */
static void _check_and_free(char *buf);

/**
 * @brief Implementation of the object interface for the Security Object.
 */
static lwm2m_object_t _security_object = {
    .next           = NULL,
    .objID          = LWM2M_SECURITY_OBJECT_ID,
    .instanceList   = NULL,
    .readFunc       = _read_cb,
    .writeFunc      = _write_cb,
    .createFunc     = _create_cb,
    .deleteFunc     = _delete_cb,
    .executeFunc    = NULL,
    .discoverFunc   = NULL,
    .userData       = NULL,
};

/**
 * @brief Credential tag counter
 */
static credman_tag_t _tag_count = CONFIG_LWM2M_CREDMAN_TAG_BASE;

static inline void _check_and_free(char *buf)
{
    if (buf) {
        lwm2m_free((void *)buf);
    }
}

static int _free_and_copy_buffer(char *dst, uint8_t *src, size_t len)
{
    /* if there was already a value, free the buffer */
    _check_and_free(dst);

    /* allocate new buffer and copy the new value */
    dst = (char *)lwm2m_malloc(len);
    if (dst) {
        memcpy(dst, (char *)src, len);
        return COAP_204_CHANGED;
    }

    return COAP_500_INTERNAL_SERVER_ERROR;
}

static void _update_credential(lwm2m_obj_security_inst_t *instance)
{
    credman_credential_t cred;
    int res;

    if (instance->cred_tag != CREDMAN_TAG_EMPTY) {
        /* a credential has already been assigned, we need to modify it */
        DEBUG("[security:update_cred] getting existing credential\n");
        res = credman_get(&cred, instance->cred_tag, CREDMAN_TYPE_PSK);
        if (res != CREDMAN_OK) {
            DEBUG("[security:update_cred] not found\n");
            return;
        }
        credman_delete(cred.tag, cred.type);
    }
    else if (!instance->secret_key) {
        /* only assign a new credential when we have a key */
        DEBUG("[security:update_cred] not assigning credential, no key available\n");
        return;
    }
    else {
        /* we are assigning a new credential, so get a new tag */
        DEBUG("[security:update_cred] assigning new credential\n");
        _tag_count++;
        cred.tag = _tag_count;
        cred.type = CREDMAN_TYPE_PSK;
    }

    /* build new credential / modify existing credential */
    cred.params.psk.id.s = instance->pub_key_or_id;
    cred.params.psk.id.len = instance->pub_key_or_id_len;
    cred.params.psk.key.s = instance->secret_key;
    cred.params.psk.key.len = instance->secret_key_len;

    /* try to register the credential */
    res = credman_add(&cred);
    if (res != CREDMAN_OK) {
        DEBUG("[security:update_cred] could not create new credential\n");
        instance->cred_tag = CREDMAN_TAG_EMPTY;
        return;
    }

    instance->cred_tag = cred.tag;
}

static int _set_cred_value(lwm2m_data_t *data, lwm2m_obj_security_inst_t *instance)
{

    /* check that the resource ID is credential-related */
    if ((LWM2M_SECURITY_PUBLIC_KEY_ID != data->id &&
         LWM2M_SECURITY_SERVER_PUBLIC_KEY_ID != data->id &&
         LWM2M_SECURITY_SECRET_KEY_ID != data->id) ||
        !IS_USED(MODULE_WAKAAMA_CLIENT_DTLS)) {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    if (instance->security_mode != LWM2M_SECURITY_MODE_PRE_SHARED_KEY) {
        DEBUG("[security:set_cred] only PSK mode supported\n");
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    int res;

    switch (data->id) {
        case LWM2M_SECURITY_PUBLIC_KEY_ID:
            /* PSK Identity */
            DEBUG("[security:set_cred] setting PSK ID: %*.s\n", data->value.asBuffer.length,
                  data->value.asBuffer.buffer);
            res = _free_and_copy_buffer(instance->pub_key_or_id, data->value.asBuffer.buffer,
                                        data->value.asBuffer.length);
            if (res == COAP_204_CHANGED) {
                instance->pub_key_or_id_len = data->value.asBuffer.length;
            }
            break;

        case LWM2M_SECURITY_SECRET_KEY_ID:
            /* PSK Key */
            DEBUG("[security:set_cred] setting PSK Key: %*.s\n", data->value.asBuffer.length,
                  data->value.asBuffer.buffer);
            res = _free_and_copy_buffer(instance->secret_key, data->value.asBuffer.buffer,
                                        data->value.asBuffer.length);
            if (res == COAP_204_CHANGED) {
                instance->secret_key_len = data->value.asBuffer.length;
            }
            break;

        case LWM2M_SECURITY_SERVER_PUBLIC_KEY_ID:
            /* TODO: extend for RPK mode */
        default:
            return COAP_500_INTERNAL_SERVER_ERROR;
    }

    /* update registered credential with the new data */
    _update_credential(instance);

    return res;
}

static int _get_value(lwm2m_data_t *data, lwm2m_obj_security_inst_t *instance)
{
    /* resource IDs are defined by Wakaama in liblwm2m.h */
    switch (data->id) {
        case LWM2M_SECURITY_URI_ID:
            lwm2m_data_encode_string(instance->uri, data);
            break;

        case LWM2M_SECURITY_BOOTSTRAP_ID:
            lwm2m_data_encode_bool(instance->is_bootstrap, data);
            break;

        case LWM2M_SECURITY_SMS_SECURITY_ID:
            lwm2m_data_encode_int(instance->sms_security_mode, data);
            break;

        case LWM2M_SECURITY_SMS_KEY_PARAM_ID:
            lwm2m_data_encode_opaque((uint8_t*)instance->sms_params, instance->sms_params_len, data);
            break;

        case LWM2M_SECURITY_SMS_SECRET_KEY_ID:
            lwm2m_data_encode_opaque((uint8_t*)instance->sms_secret, instance->sms_secret_len, data);
            break;

        case LWM2M_SECURITY_SMS_SERVER_NUMBER_ID:
            lwm2m_data_encode_int(0, data);
            break;

        case LWM2M_SECURITY_SHORT_SERVER_ID:
            lwm2m_data_encode_int(instance->short_id, data);
            break;

        case LWM2M_SECURITY_HOLD_OFF_ID:
            lwm2m_data_encode_int(instance->client_hold_off_time, data);
            break;

        case LWM2M_SECURITY_BOOTSTRAP_TIMEOUT_ID:
            lwm2m_data_encode_int(instance->bs_account_timeout, data);
            break;

        case LWM2M_SECURITY_SECURITY_ID:
            lwm2m_data_encode_int(instance->security_mode, data);
            break;

        case LWM2M_SECURITY_PUBLIC_KEY_ID:
            lwm2m_data_encode_opaque((uint8_t *)instance->pub_key_or_id,
                                     instance->pub_key_or_id_len, data);
            break;

        case LWM2M_SECURITY_SERVER_PUBLIC_KEY_ID:
            lwm2m_data_encode_opaque((uint8_t *)instance->server_pub_key,
                                     instance->server_pub_key_len, data);
            break;

        case LWM2M_SECURITY_SECRET_KEY_ID:
            lwm2m_data_encode_opaque((uint8_t *)instance->secret_key,
                                     instance->secret_key_len, data);
            break;

        default:
            return COAP_404_NOT_FOUND;
    }
    return COAP_205_CONTENT;
}

static uint8_t _read_cb(uint16_t instance_id, int *num_data, lwm2m_data_t **data_array,
                        lwm2m_object_t *object)
{
    lwm2m_obj_security_inst_t *instance;
    uint8_t result;
    int i = 0;

    /* try to get the requested instance from the object list */
    instance = (lwm2m_obj_security_inst_t *)lwm2m_list_find(object->instanceList, instance_id);
    if (NULL == instance) {
        result = COAP_404_NOT_FOUND;
        goto out;
    }

    /* if the number of resources is not specified, we need to read all resources */
    if (!*num_data) {
        DEBUG("[security:read] all resources are read\n");

        uint16_t resList[] = {
            LWM2M_SECURITY_URI_ID,
            LWM2M_SECURITY_BOOTSTRAP_ID,
            LWM2M_SECURITY_SECURITY_ID,
            LWM2M_SECURITY_PUBLIC_KEY_ID,
            LWM2M_SECURITY_SERVER_PUBLIC_KEY_ID,
            LWM2M_SECURITY_SECRET_KEY_ID,
            LWM2M_SECURITY_SMS_SECURITY_ID,
            LWM2M_SECURITY_SMS_KEY_PARAM_ID,
            LWM2M_SECURITY_SMS_SECRET_KEY_ID,
            LWM2M_SECURITY_SMS_SERVER_NUMBER_ID,
            LWM2M_SECURITY_SHORT_SERVER_ID,
            LWM2M_SECURITY_HOLD_OFF_ID,
            LWM2M_SECURITY_BOOTSTRAP_TIMEOUT_ID
        };

        /* try to allocate data structures for all resources */
        int resNum = ARRAY_SIZE(resList);
        *data_array = lwm2m_data_new(resNum);

        if (NULL == data_array) {
            result = COAP_500_INTERNAL_SERVER_ERROR;
            goto out;
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

out:
    return result;
}

static uint8_t _write_cb(uint16_t instance_id, int num_data, lwm2m_data_t *data_array,
                         lwm2m_object_t *object)
{
    lwm2m_obj_security_inst_t *instance;
    int64_t value;
    uint8_t result;
    int i = 0;

    /* try to get the requested instance from the object list */
    instance = (lwm2m_obj_security_inst_t *)lwm2m_list_find(object->instanceList, instance_id);
    if (!instance) {
        result = COAP_404_NOT_FOUND;
        goto out;
    }

    /* iterate over the array of data to write */
    do {
        switch (data_array[i].id) {
            case LWM2M_SECURITY_URI_ID:
                result = _free_and_copy_buffer(instance->uri, data_array[i].value.asBuffer.buffer,
                                               data_array[i].value.asBuffer.length + 1);
                break;

            case LWM2M_SECURITY_BOOTSTRAP_ID:
                if (1 == lwm2m_data_decode_bool(&data_array[i], &(instance->is_bootstrap))) {
                    result = COAP_204_CHANGED;
                }
                else {
                    result = COAP_400_BAD_REQUEST;
                }
                break;

            case LWM2M_SECURITY_SECURITY_ID:
                if (1 == lwm2m_data_decode_int(&data_array[i], &value)) {
                    /* check if it is a valid security mode */
                    if (LWM2M_SECURITY_MODE_NONE == value ||
                        LWM2M_SECURITY_MODE_PRE_SHARED_KEY == value ||
                        LWM2M_SECURITY_MODE_RAW_PUBLIC_KEY == value ||
                        LWM2M_SECURITY_MODE_CERTIFICATE == value)
                    {
                        instance->security_mode = value;
                        result = COAP_204_CHANGED;
                        break;
                    }
                }

                result = COAP_400_BAD_REQUEST;
                break;

            case LWM2M_SECURITY_SMS_SECURITY_ID:
                if (1 == lwm2m_data_decode_int(&data_array[i], &value)) {
                    /* check valid SMS security mode */
                    if (LWM2M_SMS_SECURITY_MODE_DTLS == value ||
                        LWM2M_SMS_SECURITY_MODE_SECURE_PACKET_STRUCT == value ||
                        LWM2M_SMS_SECURITY_MODE_NONE == value ||
                        (LWM2M_SMS_SECURITY_MODE_PROPRIETARY_START <= value &&
                         LWM2M_SMS_SECURITY_MODE_PROPRIETARY_END >= value)) {
                        instance->sms_security_mode = value;
                        result = COAP_204_CHANGED;
                        break;
                    }
                }
                result = COAP_400_BAD_REQUEST;
                break;

            case LWM2M_SECURITY_SMS_KEY_PARAM_ID:
                result = _free_and_copy_buffer(instance->sms_params,
                                               data_array[i].value.asBuffer.buffer,
                                               data_array[i].value.asBuffer.length);

                if (COAP_204_CHANGED == result) {
                    instance->sms_params_len = data_array[i].value.asBuffer.length;
                }
                break;

            case LWM2M_SECURITY_SMS_SECRET_KEY_ID:
                result = _free_and_copy_buffer(instance->sms_secret,
                                               data_array[i].value.asBuffer.buffer,
                                               data_array[i].value.asBuffer.length);

                if (COAP_204_CHANGED == result) {
                    instance->sms_secret_len = data_array[i].value.asBuffer.length;
                }
                break;

            case LWM2M_SECURITY_SMS_SERVER_NUMBER_ID:
                result = _free_and_copy_buffer(instance->sms_server_number,
                                               data_array[i].value.asBuffer.buffer,
                                               data_array[i].value.asBuffer.length);
                break;

            case LWM2M_SECURITY_SHORT_SERVER_ID:
                if (1 == lwm2m_data_decode_int(&data_array[i], &value)) {
                    /* check valid range of value */
                    if (value > 0 && value < UINT16_MAX) {
                        instance->short_id = value;
                        result = COAP_204_CHANGED;
                        break;
                    }
                }
                result = COAP_400_BAD_REQUEST;
                break;

            case LWM2M_SECURITY_HOLD_OFF_ID:
                if (1 == lwm2m_data_decode_int(&data_array[i], &value)) {
                    /* check valid range of value */
                    if (value >= 0 && value <= UINT32_MAX) {
                        instance->client_hold_off_time = value;
                        result = COAP_204_CHANGED;
                        break;
                    }
                }
                result = COAP_400_BAD_REQUEST;
                break;

            case LWM2M_SECURITY_BOOTSTRAP_TIMEOUT_ID:
                if (1 == lwm2m_data_decode_int(&data_array[i], &value)) {
                    /* check valid range of value */
                    if (value >= 0 && value <= UINT32_MAX) {
                        instance->bs_account_timeout = value;
                        result = COAP_204_CHANGED;
                        break;
                    }
                }
                result = COAP_400_BAD_REQUEST;
                break;

            case LWM2M_SECURITY_PUBLIC_KEY_ID:
            case LWM2M_SECURITY_SERVER_PUBLIC_KEY_ID:
            case LWM2M_SECURITY_SECRET_KEY_ID:
                result = _set_cred_value(&data_array[i], instance);
                break;

            default:
                result = COAP_400_BAD_REQUEST;
        }
        i++;
    } while (i < num_data && result == COAP_204_CHANGED);

out:
    return result;
}

static uint8_t _delete_cb(uint16_t instance_id, lwm2m_object_t *object)
{
    lwm2m_obj_security_inst_t *instance;

    /* try to remove the requested instance from the list */
    object->instanceList = LWM2M_LIST_RM(object->instanceList, instance_id, &instance);

    /* check if the instance was found */
    if (NULL == instance) {
        return COAP_404_NOT_FOUND;
    }

    credman_type_t type = instance->security_mode == LWM2M_SECURITY_MODE_PRE_SHARED_KEY ?
                            CREDMAN_TYPE_PSK : CREDMAN_TYPE_ECDSA;

    /* free related resources */
    credman_delete(instance->cred_tag, type);
    _check_and_free(instance->uri);
    _check_and_free(instance->pub_key_or_id);
    _check_and_free(instance->secret_key);
    _check_and_free(instance->server_pub_key);
    _check_and_free(instance->sms_params);
    _check_and_free(instance->sms_secret);
    _check_and_free(instance->sms_server_number);

    lwm2m_free(instance);

    return COAP_202_DELETED;
}

static uint8_t _create_cb(uint16_t instance_id, int num_data,
                          lwm2m_data_t * data_array,
                          lwm2m_object_t * object)
{
    lwm2m_obj_security_inst_t *instance;
    uint8_t result;

    /* try to allocate a new instance */
    instance = (lwm2m_obj_security_inst_t *)lwm2m_malloc(sizeof(lwm2m_obj_security_inst_t));
    if (!instance) {
         return COAP_500_INTERNAL_SERVER_ERROR;
    }

    memset(instance, 0, sizeof(lwm2m_obj_security_inst_t));
    instance->cred_tag = CREDMAN_TAG_EMPTY;

    /* add to the object instance list */
    instance->list.id = instance_id;
    object->instanceList = LWM2M_LIST_ADD(object->instanceList, instance);

    /* write incoming data to the instance */
    result = _write_cb(instance_id, num_data, data_array, object);

    if (result != COAP_204_CHANGED) {
        _delete_cb(instance_id, object);
    }
    else {
        result = COAP_201_CREATED;
    }

    return result;
}

lwm2m_object_t *lwm2m_object_security_get(void)
{
    return &_security_object;
}

int lwm2m_object_security_instance_create(lwm2m_object_t *object,
                                          uint16_t instance_id,
                                          uint16_t server_id,
                                          const char *server_uri,
                                          uint8_t security_mode,
                                          const credman_credential_t *cred,
                                          bool is_bootstrap,
                                          uint32_t client_hold_off_time,
                                          uint32_t bootstrap_account_timeout)
{

    lwm2m_obj_security_inst_t *instance = NULL;

    /* some sanity checks */
    if (!object || !server_id || !server_uri ||
        object->objID != LWM2M_SECURITY_OBJECT_ID) {
        goto out;
    }

    /* try to allocate an instance */
    instance = (lwm2m_obj_security_inst_t *)lwm2m_malloc(sizeof(lwm2m_obj_security_inst_t));
    if (!instance) {
        DEBUG("[lwm2m_security_object]: can't allocate new instance\n");
        goto out;
    }

    memset(instance, 0, sizeof(lwm2m_obj_security_inst_t));

    instance->list.id = instance_id;
    instance->short_id = server_id;
    instance->security_mode = security_mode;
    instance->is_bootstrap = is_bootstrap;
    instance->client_hold_off_time = client_hold_off_time;
    instance->bs_account_timeout = bootstrap_account_timeout;
    instance->cred_tag = CREDMAN_TAG_EMPTY;

    /* make a copy of the URI locally */
    instance->uri = lwm2m_strdup(server_uri);
    if (!instance->uri) {
        DEBUG("[lwm2m_security_object]: can't copy URI\n");
        goto free_instance_out;
    }

    if (IS_USED(MODULE_WAKAAMA_CLIENT_DTLS)) {
        if (security_mode != LWM2M_SECURITY_MODE_NONE) {
            /* we need a valid credential */
            if (!cred) {
                DEBUG("[lwm2m_security_object]: no credential provided\n");
                goto free_out;
            }

            if (LWM2M_SECURITY_MODE_PRE_SHARED_KEY == security_mode) {
                if (cred->type != CREDMAN_TYPE_PSK) {
                    DEBUG("[lwm2m_security_object]: Incorrect credential type for PSK mode\n");
                    goto free_out;
                }

                /* allocate buffers to hold the keys and load them */
                instance->pub_key_or_id = lwm2m_malloc(cred->params.psk.id.len);
                if (!instance->pub_key_or_id) {
                    DEBUG("[lwm2m_security_object]: can't allocate PSK ID buffer\n");
                    goto free_out;
                }

                memcpy(instance->pub_key_or_id, cred->params.psk.id.s, cred->params.psk.id.len);
                instance->pub_key_or_id_len = cred->params.psk.id.len;

                instance->secret_key = lwm2m_malloc(cred->params.psk.key.len);
                if (!instance->secret_key) {
                    DEBUG("[lwm2m_security_object]: can't allocate PSK key buffer\n");
                    lwm2m_free((void *)instance->pub_key_or_id);
                    goto free_out;
                }

                memcpy(instance->secret_key, cred->params.psk.key.s, cred->params.psk.key.len);
                instance->secret_key_len = cred->params.psk.key.len;

                /* associate a credential */
                _update_credential(instance);
                if (instance->cred_tag == CREDMAN_TAG_EMPTY) {
                    DEBUG("[lwm2m_security_object]: could not register the credential\n");
                    lwm2m_free((void *)instance->pub_key_or_id);
                    lwm2m_free((void *)instance->secret_key);
                    goto free_out;
                }
            }
            else if (LWM2M_SECURITY_MODE_RAW_PUBLIC_KEY == security_mode) {
                // TODO: add support for RPK
                DEBUG("[lwm2m_security_object]: RPK not supported yet\n");
                goto free_out;
            }
            else {
                DEBUG("[lwm2m_security_object]: Certificate mode not supported\n");
                goto free_out;
            }
        }
    }

    DEBUG("Added sec instance with URI: %s\n", instance->uri);

    /* add the new instance to the list */
    object->instanceList = LWM2M_LIST_ADD(object->instanceList, instance);
    goto out;

free_out:
    lwm2m_free(instance->uri);
free_instance_out:
    lwm2m_free(instance);
    instance = NULL;
out:
    return instance == NULL ? -1 : 0;
}
