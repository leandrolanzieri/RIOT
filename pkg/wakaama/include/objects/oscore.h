/*
 * Copyright (C) 2021 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     lwm2m_objects
 * @defgroup    lwm2m_objects_oscore OSCORE LwM2M object
 * @brief       OSCORE object implementation for LwM2M client using Wakaama
 *
 * @{
 *
 * @file
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */
#ifndef OBJECTS_OSCORE_H
#define OBJECTS_OSCORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "liblwm2m.h"
#include "lwm2m_client_config.h"
#include "oscore.h"

/**
 * @brief Algorithm Values for AES-CCM. Taken from Table 10 of RFC 8152
 * @see https://datatracker.ietf.org/doc/html/rfc8152
 */
typedef enum aead_alg {
    /**
     * @brief AES-CCM 128-bit key, 64-bit tag, 13-byte nonce.
     */
    AEAD_ALG_AES_CCM_16_64_128  = 10,
//    AEAD_ALG_AES_CCM_16_64_256  = 11,
//    AEAD_ALG_AES_CCM_64_64_128  = 12,
//    AEAD_ALG_AES_CCM_64_64_256  = 13,
//    AEAD_ALG_AES_CCM_16_128_128 = 30,
//    AEAD_ALG_AES_CCM_16_128_256 = 31,
//    AEAD_ALG_AES_CCM_64_128_128 = 32,
//    AEAD_ALG_AES_CCM_64_128_256 = 33,
} aead_alg_t;

/**
 * @brief Algorithm Values for HMAC. Taken from Table 7 of RFC 8152
 * @see https://datatracker.ietf.org/doc/html/rfc8152
 */
typedef enum hmac_alg {
    HMAC_ALG_SHA_256  = 0, /**< HMAC with SHA-256 */
} hmac_alg_t;

/**
 * @brief LwM2M OSCORE object ID.
 */
#define LWM2M_OSCORE_OBJECT_ID 21

/**
 * @name    LwM2M OSCORE object resource IDs.
 * @{
 */
#define LWM2M_OSCORE_MASTER_SECRET_ID 0 /**< Master Secret resource */
#define LWM2M_OSCORE_SENDER_ID        1 /**< Sender ID resource */
#define LWM2M_OSCORE_RECIPIENT_ID     2 /**< Recipient ID resource */
#define LWM2M_OSCORE_AEAD_ALG_ID      3 /**< AEAD algorithm resource */
#define LWM2M_OSCORE_HMAC_ALG_ID      4 /**< HMAC algorithm resource */
#define LWM2M_OSCORE_MASTER_SALT_ID   5 /**< Master Salt resource */
#define LWM2M_OSCORE_ID_CONTEXT_ID    6 /**< ID context resource */
/** @} */

/**
 * @brief Maximum number of instances of the OSCORE object
 */
#ifndef CONFIG_LWM2M_OBJ_OSCORE_INSTANCES_MAX
#define CONFIG_LWM2M_OBJ_OSCORE_INSTANCES_MAX              (2)
#endif

/**
 * @brief Buffer size for OSCORE credentials, in bytes.
 */
#ifndef CONFIG_LWM2M_OSCORE_OBJ_BUF_SIZE
#define CONFIG_LWM2M_OSCORE_OBJ_BUF_SIZE 16
#endif

/**
 * @brief Arguments for a new OSCORE object instance creation
 *        (@ref lwm2m_object_oscore_instance_create).
 */
typedef struct lwm2m_obj_oscore_args {
    const char *master_secret;
    uint32_t master_secret_len;
    const char *sender_id;
    uint32_t sender_id_len;
    const char *recipient_id;
    uint32_t recipient_id_len;
    const char *master_salt;
    uint32_t master_salt_len;
    const char *id_ctx;
    uint32_t id_ctx_len;
    aead_alg_t aead_algorithm;
    hmac_alg_t hmac_algorithm;
} lwm2m_obj_oscore_args_t;

/**
 * @brief   Get the LwM2M OSCORE object handle
 *
 * @return Pointer to the global handle of the OSCORE object.
 */
lwm2m_object_t *lwm2m_object_oscore_get(void);

/**
 * @brief   Create a new LwM2M OSCORE object instance and add it to the @p object list.
 *
 * @param[in, out] object                  OSCORE object handle.
 * @param[in] instance_id                  ID for the new instance.
 * @param[in] args                         Arguments for the instance.
 *
 * @return 0 on success
 * @return <0 otherwise
 */
int lwm2m_object_oscore_instance_create(lwm2m_object_t *object, uint16_t instance_id,
                                        const lwm2m_obj_oscore_args_t *args);

/**
 * @brief Get the OSCORE context of a given OSCORE object instance as a client.
 *
 * @param[in] object                        OSCORE object handle.
 * @param[in] instance_id                   ID of the object instance
 *
 * @return Pointer to the context structure
 * @retval NULL if not found
 */
struct context *lwm2m_object_oscore_get_client_ctx(lwm2m_object_t *object, uint16_t instance_id);

/**
 * @brief Get the OSCORE context of a given OSCORE object instance as a server.
 *
 * @param[in] object                        OSCORE object handle.
 * @param[in] instance_id                   ID of the object instance
 *
 * @return Pointer to the context structure
 * @retval NULL if not found
 */
struct context *lwm2m_object_oscore_get_server_ctx(lwm2m_object_t *object, uint16_t instance_id);

#ifdef __cplusplus
}
#endif

#endif /* OBJECTS_OSCORE_H */
/** @} */
