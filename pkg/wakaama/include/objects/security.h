/*
 * Copyright (C) 2021 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     lwm2m_objects
 * @defgroup    lwm2m_objects_security Security LwM2M object
 * @brief       Security object implementation for LwM2M client using Wakaama
 *
 * This implements the LwM2M Security object as specified in the Appendix E1 of
 * the LwM2M specification.
 *
 * So far only NO_SEC and PSK (Pre-shared key) modes are available.
 *
 * ## Resources
 *
 * For an XML description of the object see
 * https://raw.githubusercontent.com/OpenMobileAlliance/lwm2m-registry/prod/version_history/0-1_0.xml.
 *
 * |         Name            | ID | Mandatory |  Type   |  Range  | Units |
 * | ----------------------- | -- | --------- | ------- | ------- | ----- |
 * | Server URI              |  0 |    Yes    | String  |         |       |
 * | Bootstrap Server        |  1 |    Yes    | Boolean |         |       |
 * | Security Mode           |  2 |    Yes    | Integer |   0-3   |       |
 * | Public Key or ID        |  3 |    Yes    | Opaque  |         |       |
 * | Server Public Key or ID |  4 |    Yes    | Opaque  |         |       |
 * | Secret Key              |  5 |    Yes    | Opaque  |         |       |
 * | SMS Security Mode       |  6 |    No     | Integer |  0-255  |       |
 * | SMS Binding Key Param.  |  7 |    No     | Opaque  |   6 B   |       |
 * | SMS Binding Secret Keys |  8 |    No     | Opaque  | 32-48 B |       |
 * | Server SMS Number       |  9 |    No     | String  |         |       |
 * | Short Server ID         | 10 |    No     | Integer | 1-65535 |       |
 * | Client Hold Off Time    | 11 |    No     | Integer |         |   s   |
 * | BS Account Timeout      | 12 |    No     | Integer |         |   s   |
 *
 * ## Usage
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 * // assuming buffers psk_id and psk_key containing credential information
 * // [...]
 *
 * // create credential
 * credman_credential_t cred = {
 *      .type = CREDMAN_TYPE_PSK,
 *      .params = {
 *          .psk = {
 *              .key = { .s = psk_key, .len = sizeof(psk_key) - 1 },
 *              .id = { .s = psk_id, .len = sizeof(psk_id) - 1 },
 *          }
 *      }
 * }
 *
 * // get the security object handle
 * lwm2m_object_t *sec_obj = lwm2m_object_security_get();
 *
 * // instantiate a new security object
 * int res = lwm2m_object_security_instance_create(sec_obj,
 *                                                 1,     // instance ID
 *                                                 1,     // server short ID
 *                                                 LWM2M_SECURITY_MODE_PRE_SHARED_KEY,
 *                                                 &credential,
 *                                                 false, // security not for a bootstrap server
 *                                                 10,    // 10s of hold off time
 *                                                 0);    // ignored for normal server
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * @{
 *
 * @file
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */

#ifndef OBJECTS_SECURITY_H
#define OBJECTS_SECURITY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "liblwm2m.h"
#include "net/credman.h"
#include "lwm2m_client_config.h"

#define LWM2M_SECURITY_MODE_PRE_SHARED_KEY  0
#define LWM2M_SECURITY_MODE_RAW_PUBLIC_KEY  1
#define LWM2M_SECURITY_MODE_CERTIFICATE     2
#define LWM2M_SECURITY_MODE_NONE            3

/**
 * @brief SMS binding security mode values.
 */
enum lwm2m_security_sms_sec_mode {
    LWM2M_SMS_SECURITY_MODE_DTLS = 1,                   /**< DTLS mode (Device terminated) */
    LWM2M_SMS_SECURITY_MODE_SECURE_PACKET_STRUCT = 2,   /**< Secure Packet Structure */
    LWM2M_SMS_SECURITY_MODE_NONE = 3,                   /**< NoSec */
    LWM2M_SMS_SECURITY_MODE_PROPRIETARY_START = 204,    /**< Start range of proprietary modes */
    LWM2M_SMS_SECURITY_MODE_PROPRIETARY_END = 255,      /**< End range of proprietary modes */
};

/**
 * @brief   Get the LwM2M Security object handle
 *
 * @return Pointer to the global handle of the security object.
 */
lwm2m_object_t *lwm2m_object_security_get(void);

/**
 * @brief   Create a new LwM2M Security object instance and add it to the @p object list.
 *
 * @param[in, out] object                  Security object handle.
 * @param[in] instance_id                  ID for the new instance.
 * @param[in] server_id                    Server's short ID the instance is associated to.
 * @param[in] server_uri                   Server's URI the instance is associated to. Note that a
 *                                         copy of the URI will be kept locally.
 * @param[in] security_mode                Security mode to use. Valid values are
 *                                         LWM2M_SECURITY_MODE_NONE and
 *                                         LWM2M_SECURITY_MODE_PRE_SHARED_KEY.
 * @param[in] cred                         Pointer to an initialized credential when a security mode
 *                                         other than LWM2M_SECURITY_MODE_NONE is used. The tag
 *                                         is ignored. For now only @ref CREDMAN_TYPE_PSK type is
 *                                         supported.
 * @param[in] is_bootstrap                 When `true` this instance is associated to the
 *                                         Bootstrap-Server.
 * @param[in] client_hold_off_time         Time, in seconds, to wait before initiating a 'Client
 *                                         Initiated Bootstrap', after it has been determined that
 *                                         it should be initiated.
 * @param[in] bootstrap_account_timeout    Time, in seconds, that the client waits before it purges
 *                                         the Bootstrap-Server's account. 0 means never.
 *
 * @return 0 on success
 * @return <0 otherwise
 */
int lwm2m_object_security_instance_create(lwm2m_object_t *object,
                                          uint16_t instance_id,
                                          uint16_t server_id,
                                          const char *server_uri,
                                          uint8_t security_mode,
                                          const credman_credential_t *cred,
                                          bool is_bootstrap,
                                          uint32_t client_hold_off_time,
                                          uint32_t bootstrap_account_timeout);

#ifdef __cplusplus
}
#endif

#endif /* OBJECTS_SECURITY_H */
/** @} */
