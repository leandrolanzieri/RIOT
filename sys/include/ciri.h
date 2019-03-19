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

#include "cn-cbor/cn-cbor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    //CIRI_OPT_BEGIN = 0,
    CIRI_OPT_SCHEME = 1,
    CIRI_OPT_HOST_NAME = 2,
    CIRI_OPT_HOST_IP = 3,
    CIRI_OPT_PORT = 4,
    CIRI_OPT_PATH_TYPE = 5,
    CIRI_OPT_PATH = 6,
    CIRI_OPT_QUERY = 7,
    CIRI_OPT_FRAGMENT = 8
    //CIRI_OPT_END = 9
} ciri_opt_type_t;

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
        char *string;
        uint16_t integer;
        ciri_path_type_t path_type;
    }v;
} ciri_opt_t;

int ciri_check_format(ciri_opt_t *href);

int ciri_check_abs(ciri_opt_t *href);

int ciri_check_rel(ciri_opt_t *href);

int ciri_resolve(ciri_opt_t *base, ciri_opt_t *href, unsigned rel);

int ciri_recompose(ciri_opt_t *href, uint8_t *buf, size_t buf_len);

int ciri_decompose(ciri_opt_t *href, size_t href_len, uint8_t *buf,
                   size_t buf_len);

int ciri_coap(ciri_opt_t *href, uint8_t *buf, size_t buf_len);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* CIRI_H */
