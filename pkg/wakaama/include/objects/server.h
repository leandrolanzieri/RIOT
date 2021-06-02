/*
 * Copyright (C) 2021 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     lwm2m_objects
 * @defgroup    lwm2m_objects_server Server LwM2M object
 * @brief       Server object implementation for LwM2M client using Wakaama
 *
 * This implements the LwM2M Access Control object (ID 1) as specified in the LwM2M registry.
 *
 * This LwM2M Objects provides the data related to a LwM2M Server. A Bootstrap-Server has no such an
 * Object Instance associated to it. This object is mandatory.
 *
 * ## Resources
 *
 * For an XML description of the object see
 * https://raw.githubusercontent.com/OpenMobileAlliance/lwm2m-registry/prod/version_history/1-1_0.xml
 *
 * | Name                        | ID | Mandatory |   Type  |     Range    | Units | Implemented | Multiple |
 * |-----------------------------|:--:|:---------:|:-------:|:------------:|:-----:|:-----------:|----------|
 * | Short Server ID             | 0  | Yes       | Integer | 1 - 65535    | -     | Yes         |    No    |
 * | Lifetime                    | 1  | Yes       | Integer | -            | s     | Yes         |    No    |
 * | Default Minimum Period      | 2  | No        | Integer | -            | s     | Yes         |    No    |
 * | Default Maximum Period      | 3  | No        | Integer | -            | s     | Yes         |    No    |
 * | Disable                     | 4  | No        | -       | -            | -     | Yes         |    No    |
 * | Disable Timeout             | 5  | No        | Integer | -            | s     | Yes         |    No    |
 * | Notification storing        | 6  | Yes       | Boolean | -            | -     | Yes         |    No    |
 * | Binding                     | 7  | Yes       | String  | sec. 5.3.1.1 | -     | Yes         |    No    |
 * | Registration update trigger | 8  | Yes       | -       | -            | -     | Yes         |    No    |
 *
 * ## Usage
 *
 * Each LwM2M server account is conformed at least by one instance of the Server object and one
 * instance of the @ref lwm2m_objects_security. These two are associated by the Server's short ID.
 * The Security object holds information such as credentials and URI, while the Server object holds
 * information such as the lifetime of the registration and notification default periods.
 *
 * Make sure that every Server object instance has a corresponding Security object instance with the
 * same server short ID (see @ref lwm2m_obj_security_args_t::server_id).
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 *  // assuming a Security instance exists, with the server short ID 1
 *  // [...]
 *
 *  // prepare instance arguments
 *  lwm2m_obj_server_args_t server_args = {
 *       .short_id = 1,             // must match the one use in the security instance
 *       .binding = BINDING_U,
 *       .lifetime = 120,           // two minutes
 *       .max_period = 120,
 *       .min_period = 60,
 *       .notification_storing = false,
 *       .disable_timeout = 3600    // one hour
 *   };
 *
 *  // get the server object handle
 *  lwm2m_object_t *server_obj = lwm2m_object_server_get();
 *
 *  // instantiate a new server object
 *  res = lwm2m_object_server_instance_create(server_obj, 0, &server_args);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Support for multiple server connections is available. For that, just instantiate one instance of
 * Security and Server object for each server. When using DTLS some configurations may need
 * adaption, depending on the chosen backend (e.g. maximum number of peers).
 *
 * @{
 *
 * @file
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */

#ifndef OBJECTS_SERVER_H
#define OBJECTS_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "liblwm2m.h"

/**
 * @defgroup lwm2m_objects_server_conf  Server LwM2M object compile configurations
 * @{
 */
/**
 * @brief   Maximum number of Server object instances
 */
#ifndef CONFIG_LWM2M_SERVER_INSTANCES_MAX
#define CONFIG_LWM2M_SERVER_INSTANCES_MAX      2
#endif
/** @} */

/* these macros are defined in liblwm2m.h, reproduced here just for documentation purposes */
#if defined(DOXYGEN)

/**
 * @brief LwM2M Client Object ID
 */
#define LWM2M_CLIENT_OBJECT_ID          11001 // TODO: to be defined

/**
 * @brief Server LwM2M object ID.
 */
#define LWM2M_SERVER_OBJECT_ID          1

/**
 * @name Server object resource IDs.
 * @{
 */
/**
 * @brief Short server ID.
 */
#define LWM2M_SERVER_SHORT_ID_ID        0

/**
 * @brief Lifetime ID.
 */
#define LWM2M_SERVER_LIFETIME_ID        1

/**
 * @brief Default minimum period ID.
 */
#define LWM2M_SERVER_MIN_PERIOD_ID      2

/**
 * @brief Default maximum period ID.
 */
#define LWM2M_SERVER_MAX_PERIOD_ID      3

/**
 * @brief Disable ID.
 */
#define LWM2M_SERVER_DISABLE_ID         4

/**
 * @brief Disable timeout ID.
 */
#define LWM2M_SERVER_TIMEOUT_ID         5

/**
 * @brief Notification storing ID.
 */
#define LWM2M_SERVER_STORING_ID         6

/**
 * @brief Binding ID.
 */
#define LWM2M_SERVER_BINDING_ID         7

/**
 * @brief Registration update trigger ID.
 */
#define LWM2M_SERVER_UPDATE_ID          8
/** @} */

/**
 * @brief Possible transport bindings.
 */
typedef enum {
    BINDING_UNKNOWN = 0,    /**< Unknown */
    BINDING_U,              /**< UDP */
    BINDING_UQ,             /**< UDP queue mode */
    BINDING_S,              /**< SMS */
    BINDING_SQ,             /**< SMS queue mode */
    BINDING_US,             /**< UDP plus SMS */
    BINDING_UQS             /**< UDP queue mode plus SMS */
} lwm2m_binding_t;

#endif /* DOXYGEN */

/**
 * @brief Arguments for the creation of a Server object instance.
 */
struct lwm2m_obj_server_args {
    uint16_t short_id;              /**< short server ID */
    uint32_t lifetime;              /**< lifetime */
    uint32_t min_period;            /**< default minimum observation period */
    uint32_t max_period;            /**< default maximum observation period */
    uint32_t disable_timeout;       /**< period to disable the LwM2M server */
    bool notification_storing;      /**< store "Notify" operations */
    lwm2m_binding_t binding;        /**< transport binding */
};

typedef struct lwm2m_obj_server_args lwm2m_obj_server_args_t;
typedef struct lwm2m_obj_server_args lwm2m_obj_client_args_t;

/**
 * @brief   Get the Server object handle
 *
 * @return Pointer to the global handle of the Server object.
 */
lwm2m_object_t *lwm2m_object_server_get(void);

/**
 * @brief   Get the Client object handle
 *
 * @return Pointer to the global handle of the Client object.
 */
lwm2m_object_t *lwm2m_object_client_get(void);

/**
 * @brief   Create a server object instance.
 *
 * @pre `object != NULL && args != NULL`
 *
 * @param[in] object        Server object handle
 * @param[in] instance_id   ID for the new instance
 * @param[in] args          Creation arguments
 *
 * @retval 0 on success
 * @retval <0 otherwise
 */
int lwm2m_object_server_instance_create(lwm2m_object_t *object, uint16_t instance_id,
                                        const lwm2m_obj_server_args_t *args);

int lwm2m_object_client_instance_create(lwm2m_object_t *object, uint16_t instance_id,
                                        const lwm2m_obj_client_args_t *args);

#ifdef __cplusplus
}
#endif

#endif /* OBJECTS_SERVER_H */
/** @} */
