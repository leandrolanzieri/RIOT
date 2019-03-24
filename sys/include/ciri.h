/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_coral CoRAL encoder decoder
 * @ingroup     sys_serialization
 * @brief       CoRE CoRAL encoder and decoder
 * @{
 *
 * @brief       encoding and decoding functions for CoRE CoRAL
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */

#ifndef CIRI_H
#define CIRI_H

#include <stddef.h> /* for size_t */
#include <stdint.h>
#include "sys/types.h"
#include "net/ipv6/addr.h"

#include "cn-cbor/cn-cbor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CIRI_RET_OK  (0)
#define CIRI_RET_ERR (1)

typedef enum {
    CIRI_OPT_BEGIN = 0,
    CIRI_OPT_SCHEME = 1,
    CIRI_OPT_HOST_NAME = 2,
    CIRI_OPT_HOST_IP = 3,
    CIRI_OPT_PORT = 4,
    CIRI_OPT_PATH_TYPE = 5,
    CIRI_OPT_PATH = 6,
    CIRI_OPT_QUERY = 7,
    CIRI_OPT_FRAGMENT = 8,
    CIRI_OPT_END = 9
} ciri_opt_type_t;

// TODO: Shared types with CoRAL, should integrate at some point
typedef struct {
    const char *str;
    size_t len;
} ciri_str_t;

typedef enum {
    CIRI_PATH_ABS = 0,
    CIRI_PATH_APPEND_REL = 1,
    CIRI_PATH_APPEND = 2,
    CIRI_PATH_REL = 3
    // should define everything?
} ciri_path_type_t;

typedef struct ciri_opt {
    ciri_opt_type_t type;
    struct ciri_opt *next;
    union {
        ciri_str_t string;
        ipv6_addr_t ip;
        ciri_path_type_t path_type;
        uint16_t integer;
    }v;
} ciri_opt_t;

int ciri_is_well_formed(ciri_opt_t *href);

int ciri_is_absolute(ciri_opt_t *href);

int ciri_is_relative(ciri_opt_t *href);

int ciri_resolve(ciri_opt_t *base, ciri_opt_t *href, unsigned rel);

int ciri_recompose(ciri_opt_t *href, uint8_t *buf, size_t buf_len);

int ciri_decompose(ciri_opt_t *href, unsigned href_len, uint8_t *buf,
                   size_t buf_len);

int ciri_coap(ciri_opt_t *href, uint8_t *buf, size_t buf_len);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* CIRI_H */
