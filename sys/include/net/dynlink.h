/*
 * Copyright (c) 2015-2017 Ken Bannister. All rights reserved.
 *               2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @brief       gcoap definition
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef NET_DYNLINK_H
#define NET_DYNLINK_H

#include <stdint.h>

#include "net/ipv6/addr.h"
#include "net/sock/udp.h"
#include "net/nanocoap.h"
#include "xtimer.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DYNLINK_TARGET_MAX
#define DYNLINK_TARGET_MAX (IPV6_ADDR_MAX_STR_LEN + CONFIG_NANOCOAP_URI_MAX + 64) // TODO: review this value
#endif

#ifndef CONFIG_DYNLINK_CLIF_ATTRS_MAX
#define CONFIG_DYNLINK_CLIF_ATTRS_MAX     (10)
#endif

#ifndef CONFIG_DYNLINK_COND_MAX
#define CONFIG_DYNLINK_COND_MAX   (4)
#endif

#ifndef CONFIG_DYNLINK_BIND_TABLE_PATH
#define CONFIG_DYNLINK_BIND_TABLE_PATH   "/bnd"
#endif

#define DYNLINK_BIND_TABLE_RT   "core.bnd"
#define DYNLINK_BINDING_RT      "boundto"

#define DYNLINK_BIND_POLL   "poll"
#define DYNLINK_BIND_OBS    "obs"
#define DYNLINK_BIND_PUSH   "push"

typedef enum {
    DYNLINK_NONE = 0,
    DYNLINK_POLL,
    DYNLINK_OBS,
    DYNLINK_PUSH
} dynlink_bind_t;

typedef enum {
    DYNLINK_RES_TYPE_SCALAR = 0,
    DYNLINK_RES_TYPE_STRING,
    DYNLINK_RES_TYPE_BOOL
} dynlink_res_type_t;

typedef int32_t dynlink_scalar_t;

typedef union {
    dynlink_scalar_t n;
    bool b;
    char *s;
} dynlink_value_t;

typedef struct {
    dynlink_value_t v;
    dynlink_value_t last_v;
    xtimer_ticks64_t last_sync;
    dynlink_res_type_t type;
} dynlink_res_t;

typedef struct {
    char clif[DYNLINK_TARGET_MAX];
    size_t clif_len;
    const coap_resource_t *coap_res;
    unsigned st;
    int lt;
    int gt;
    bool band;
    unsigned pmin;
    unsigned pmax;
    dynlink_bind_t bind;
} dynlink_condition_t;

#ifdef __cplusplus
}
#endif

#endif /* NET_DYNLINK_H */
/** @} */
