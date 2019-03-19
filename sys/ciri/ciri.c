/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup sys_coral
 * @{
 * @file
 * @brief   Functions to encode and decode CoRE CoRAL
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

static void _print_opt(ciri_opt_t *e);
static void *_cbor_calloc(size_t count, size_t size, void *memblock);
static void _cbor_free(void *ptr, void *memblock);

static cn_cbor _block_storage_data[CIRI_CBOR_NUM_RECORDS];
static memarray_t _storage;
static cn_cbor_context _ct = {
    .calloc_func = _cbor_calloc,
    .free_func = _cbor_free,
    .context = &_storage
};

static void _print_opt(ciri_opt_t *e)
{
     printf(" Option type: ");

    switch (e->type) {
        case CIRI_OPT_SCHEME:
            printf("scheme | content: %s", e->v.string);
            break;
        // TODO add others
        default:
            printf("other");
            break;
    }
    puts("");
}

static void *_cbor_calloc(size_t count, size_t size, void *memblock)
{
    (void)count;
    void *block = memarray_alloc(memblock);
    if (block) {
        memset(block, 0, size);
    }
    return block;
}

static void _cbor_free(void *ptr, void *memblock)
{
    memarray_free(memblock, ptr);
}

