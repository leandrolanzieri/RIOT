/*
 * Copyright (C) 2021 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     lwm2m_objects
 * @defgroup    lwm2m_objects_access_control Access Control
 * @brief       Access Control object implementation for LwM2M client using Wakaama
 *
 * This implements the LwM2M Access Control object (ID 2) as specified in the LwM2M registry.
 *
 * The Access Control Object is used to check whether the LwM2M Server has access right for
 * performing an operation. It is needed only on multi-sever deployments.
 *
 * To use this object add `USEMODULE += wakaama_objects_access_control` to the application Makefile.
 *
 * ## Resources
 *
 * For an XML description of the object see
 * https://raw.githubusercontent.com/OpenMobileAlliance/lwm2m-registry/prod/version_history/2-1_0.xml
 *
 * | Name                 | ID | Mandatory |   Type  |   Range   | Units | Implemented | Multiple |
 * |----------------------|:--:|:---------:|:-------:|:---------:|:-----:|:-----------:|:--------:|
 * | Object ID            | 0  | Yes       | Integer | 1 - 65534 | -     | Yes         | No       |
 * | Object Instance ID   | 1  | Yes       | Integer | 0 - 65535 | -     | Yes         | No       |
 * | ACL                  | 2  | No        | Integer | 16 bit    | -     | Yes         | Yes      |
 * | Access Control Owner | 3  | Yes       | Integer | 0 - 65535 | -     | Yes         | No       |
 *
 * @{
 *
 * @file
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */

#ifndef OBJECTS_ACCESS_CONTROL_H
#define OBJECTS_ACCESS_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "liblwm2m.h"

/**
 * @brief LwM2M Client Access Control Object ID
 */
#define LWM2M_CLIENT_ACCESS_CONTROL_OBJECT_ID          11002 // TODO: to be defined

/**
 * @brief Maximum number of instances of the Access Control object.
 */
#ifndef CONFIG_LWM2M_ACCESS_CONTROL_INSTANCES_MAX
#define CONFIG_LWM2M_ACCESS_CONTROL_INSTANCES_MAX          (4)
#endif

/**
 * @brief Maximum number of ACLs.
 */
#ifndef CONFIG_LWM2M_ACCESS_CONTROL_ACLS_MAX
#define CONFIG_LWM2M_ACCESS_CONTROL_ACLS_MAX          (8)
#endif

/**
 * @brief Access Control object ID.
 */
#define LWM2M_ACCESS_CONTROL_OBJECT_ID         (2)

/**
 * @name Access Control object resource IDs.
 * @{
 */
/**
 * @brief Object ID resource ID.
 */
#define LWM2M_ACCESS_CONTROL_OBJ_ID_ID         (0)

/**
 * @brief Object Instance ID resource ID.
 */
#define LWM2M_ACCESS_CONTROL_OBJ_INST_ID_ID    (1)

/**
 * @brief Access Control List (ACL) resource ID.
 */
#define LWM2M_ACCESS_CONTROL_ACL_ID            (2)

/**
 * @brief Access Control Owner resource ID.
 */
#define LWM2M_ACCESS_CONTROL_AC_OWNER          (3)
/** @} */

/**
 * @name Access Control flags.
 * @{
 */
/**
 * @brief Read, Observe and Write-Attributes access.
 */
#define LWM2M_ACCESS_CONTROL_READ              (0x01)

/**
 * @brief Write access.
 */
#define LWM2M_ACCESS_CONTROL_WRITE             (0x02)

/**
 * @brief Execute access.
 */
#define LWM2M_ACCESS_CONTROL_EXECUTE           (0x04)

/**
 * @brief Delete access.
 */
#define LWM2M_ACCESS_CONTROL_DELETE            (0x08)

/**
 * @brief Create access.
 */
#define LWM2M_ACCESS_CONTROL_CREATE            (0x10)
/** @} */

/**
 * @brief Arguments for the creation of a Access Control object instance.
 */
typedef struct lwm2m_obj_access_control_args {
    uint16_t obj_id;        /**< Object ID to which the instance is applicable */
    uint16_t obj_inst_id;   /**< Object Instance ID to which the instance is applicable */
    uint16_t owner;         /**< Short Server ID of a certain LwM2M Server that manages access */
} lwm2m_obj_access_control_args_t;

/**
 * @brief Arguments for the creation of an Access Control List resource instance.
 */
typedef struct lwm2m_obj_access_control_acl_args {
    /**
     * @brief Resource instance ID. MUST match the Short Server ID of a certain LwM2M Server for
     * which associated access rights are contained in lwm2m_obj_access_control_acl_args_t::access.
     * The Resource Instance ID 0 is a specific ID determining the ACL Instance which contains the
     * default access rights.
     */
    uint16_t res_inst_id;

    /**
     * @brief Access flags. Each bit set grants an access right to the LwM2M Server to the
     * corresponding operation. See the Access Control flags.
     */
    uint16_t access;
} lwm2m_obj_access_control_acl_args_t;

/**
 * @brief   Get the Access Control object handle
 *
 * @return Pointer to the global handle of the Access Control object.
 */
lwm2m_object_t *lwm2m_object_access_control_get(void);

/**
 * @brief   Get the Client Access Control object handle
 *
 * @return Pointer to the global handle of the Client Access Control object.
 */
lwm2m_object_t *lwm2m_object_client_access_control_get(void);

/**
 * @brief   Create an access control object instance.
 *
 * @pre `object != NULL && args != NULL`
 *
 * @param[in] object        Access control object handle
 * @param[in] instance_id   ID for the new instance
 * @param[in] args          Creation arguments
 *
 * @retval 0 on success
 * @retval <0 otherwise
 */
int lwm2m_object_access_control_instance_create(lwm2m_object_t *object, uint16_t instance_id,
                                                const lwm2m_obj_access_control_args_t *args);

/**
 * @brief   Add an access control list instance to an access control object instance.
 *
 * @pre `object != NULL && args != NULL`
 *
 * @param[in] object        Access control object handle
 * @param[in] instance_id   Instance ID where to add the new ACL
 * @param[in] args          Creation arguments
 *
 * @retval 0 on success
 * @retval <0 otherwise
 */
int lwm2m_object_access_control_add(lwm2m_object_t *object, uint16_t instance_id,
                                    const lwm2m_obj_access_control_acl_args_t *args);


int lwm2m_object_access_control_get_access(uint16_t server_id, const lwm2m_uri_t *uri,
                                           lwm2m_object_t *object);

int lwm2m_object_access_control_get_owner(const lwm2m_uri_t *uri, const lwm2m_object_t *object);

#ifdef __cplusplus
}
#endif

#endif /* OBJECTS_ACCESS_CONTROL_H */
/** @} */
