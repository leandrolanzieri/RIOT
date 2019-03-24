/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup sys_ciri
 * @{
 * @file
 * @brief   Implementation of Constrained Internationalized Resource Identifiers
 *          (CIRI)
 *
 * @author  Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 *
 */

#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "cn-cbor/cn-cbor.h"
#include "memarray.h"
#include "ciri.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#define CIRI_CBOR_NUM_RECORDS  (16)

// static void *_cbor_calloc(size_t count, size_t size, void *memblock);
// static void _cbor_free(void *ptr, void *memblock);

// static cn_cbor _block_storage_data[CIRI_CBOR_NUM_RECORDS];
// static memarray_t _storage;
// static cn_cbor_context _ct = {
//     .calloc_func = _cbor_calloc,
//     .free_func = _cbor_free,
//     .context = &_storage
// };

static int _valid_transition(ciri_opt_type_t prev, ciri_opt_type_t next)
{
    switch (prev) {
        case CIRI_OPT_BEGIN:
            return ((next == CIRI_OPT_SCHEME) || (next == CIRI_OPT_HOST_NAME) ||
                    (next == CIRI_OPT_HOST_IP) || (next == CIRI_OPT_PORT) ||
                    (next == CIRI_OPT_PATH_TYPE) || (next == CIRI_OPT_PATH) ||
                    (next == CIRI_OPT_QUERY) || (next == CIRI_OPT_FRAGMENT) ||
                    (next == CIRI_OPT_END));
        case CIRI_OPT_SCHEME:
            return ((next == CIRI_OPT_HOST_NAME) || (next == CIRI_OPT_HOST_IP));
        case CIRI_OPT_HOST_NAME:
        case CIRI_OPT_HOST_IP:
            return ((next == CIRI_OPT_PORT));
        case CIRI_OPT_PORT:
        case CIRI_OPT_PATH_TYPE:
        case CIRI_OPT_PATH:
            return ((next == CIRI_OPT_PATH) || (next == CIRI_OPT_QUERY) ||
                    (next == CIRI_OPT_FRAGMENT) || next == CIRI_OPT_END);
        case CIRI_OPT_QUERY:
            return ((next == CIRI_OPT_QUERY) || (next == CIRI_OPT_FRAGMENT) ||
                    (next == CIRI_OPT_END));
        case CIRI_OPT_FRAGMENT:
            return (next == CIRI_OPT_END);
        default:
            DEBUG("Unknown option type\n");
            return 0;
    }
}

static int _valid_last_opt(ciri_opt_type_t type)
{
    return ((type == CIRI_OPT_PORT) || (type == CIRI_OPT_PATH_TYPE) ||
            (type == CIRI_OPT_PATH) || (type == CIRI_OPT_QUERY) ||
            (type == CIRI_OPT_FRAGMENT) || (type == CIRI_OPT_END));
}

int ciri_is_well_formed(ciri_opt_t *href)
{
    assert(href);
    ciri_opt_type_t prev_type = CIRI_OPT_BEGIN;
    ciri_opt_t *opt = href;
    while(opt) {
        if (!_valid_transition(prev_type, opt->type)) {
            DEBUG("Malformed CIRI, invalid transition detected\n");
            return CIRI_RET_ERR;
        }
        prev_type = opt->type;
        opt = opt->next;
    }
    if (!_valid_last_opt(prev_type)) {
        DEBUG("Malformed CIRI, invalid last option\n");
        return CIRI_RET_ERR;
    }
    return CIRI_RET_OK;
}

int ciri_is_absolute(ciri_opt_t *href)
{
    assert(href);
    if (ciri_is_well_formed(href) != CIRI_RET_OK ||
        href->type != CIRI_OPT_SCHEME) {
        return CIRI_RET_ERR;
    }
    return CIRI_RET_OK;
}

// static void *_cbor_calloc(size_t count, size_t size, void *memblock)
// {
//     (void)count;
//     void *block = memarray_alloc(memblock);
//     if (block) {
//         memset(block, 0, size);
//     }
//     return block;
// }

// static void _cbor_free(void *ptr, void *memblock)
// {
//     memarray_free(memblock, ptr);
// }

