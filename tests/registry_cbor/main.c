/*
 * Copyright (C) 2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @file
 * @brief       Shows the use of the RIOT registry for handling runtime
 *              configurations using multiple store options.
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */

#include <stdio.h>
#include <string.h>

#include "errno.h"
#include "shell.h"
#include "board.h"
#include "fmt.h"
#include "cn-cbor/cn-cbor.h"
#include "registry/registry.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#define MAX_DECODE_BUFFER   (128)

static cn_cbor_context *cb_ctx;

extern registry_config_group_t app_config_group;
cn_cbor_context *cbor_memarray_init(void);


static void _print_cbor(const cn_cbor *cb, int depth, void *context)
{
    (void)context;
    printf("--|");
    for (int i = 0; i < depth; i++) {
        printf("--|");
    }

    switch(cb->type) {
        case CN_CBOR_FALSE:
            printf("<bool>: False");
            break;
        case CN_CBOR_TRUE:
            printf("<bool>: True");
            break;
        case CN_CBOR_NULL:
            printf(": NULL");
            break;
        case CN_CBOR_UNDEF:
            printf(": undefined");
            break;
        case CN_CBOR_UINT:
            printf("<uint>: %ld", cb->v.uint);
            break;
        case CN_CBOR_INT:
            printf("<int>: %ld", cb->v.sint);
            break;
        case CN_CBOR_BYTES:
            printf("<bytes>: ");
            for (int i = 0; i < cb->length; i++) {
                printf("%2x ", cb->v.bytes[i]);
            }
            break;
        case CN_CBOR_TEXT:
            printf("<text>: %.*s", cb->length, cb->v.str);
            break;
        case CN_CBOR_ARRAY:
            printf("<array> (%d children)", cb->length);
            break;
        case CN_CBOR_MAP:
            printf("<map> (%d children)", cb->length / 2);
            break;
#ifndef CBOR_NO_FLOAT
        case CN_CBOR_FLOAT:
            printf("<float>: %f", cb->v.f);
            break;
#endif
        default:
            printf("<other>");
    }
    puts("");
}

typedef void (*cn_visit_func)(const cn_cbor *cb, int depth, void *context);
static void _visit(const cn_cbor *cb, cn_visit_func visitor, void *context) {
    const cn_cbor *p = cb;
    int depth = 0;
    while (p) {
visit:
      visitor(p, depth, context);
      if (p->first_child) {
        p = p->first_child;
        depth++;
      } else {
        if (p->next) {
          p = p->next;
        } else {
          while (p->parent) {
            depth--;
            if (p->parent->next) {
              p = p->parent->next;
              goto visit;
            }
            p = p->parent;
          }
          return;
        }
      }
    }
}

int cmd_get(int argc, char **argv)
{
    cn_cbor *config = NULL;
    uint8_t decode_buf[MAX_DECODE_BUFFER];
    if (argc == 1) {
        /* trying to get all parameters */
        config = registry_get_config(NULL, cb_ctx);
        if (!config->length) {
            DEBUG("Error: Parameter does not exist\n");
            return 1;
        }
    }
    else if (argc == 2) {
        /* trying to get a specified parameter */
        int len = fmt_hex_bytes(decode_buf, argv[1]);
        if (!len) {
            DEBUG("Error: Could not decode hex value\n");
            return 1;
        }
        cn_cbor *request = cn_cbor_decode(decode_buf, len, cb_ctx, NULL);
#if ENABLE_DEBUG
        _visit(request, _print_cbor, NULL);
#endif
        if (request->type != CN_CBOR_MAP) {
            DEBUG("Error: Malformed CBOR\n");
            return 1;
        }
        config = registry_get_config(request, cb_ctx);
        if (!config->length) {
            DEBUG("Error: Parameter does not exist\n");
            return 1;
        }
        cn_cbor_free(request, cb_ctx);
    }
    else {
        printf("usage: %s [parameter]\n", argv[0]);
        return 1;
    }

#if ENABLE_DEBUG
    puts("");
    _visit(config, _print_cbor, NULL);
    puts("");
#endif

    ssize_t enc_len = cn_cbor_encoder_write(decode_buf, 0, sizeof(decode_buf), config);
    for (int i = 0; i < enc_len; i++) {
        print_byte_hex(decode_buf[i]);
    }

    cn_cbor_free(config, cb_ctx);
    return 0;
}

int cmd_set(int argc, char **argv)
{
    if (argc != 3) {
        printf("usage: %s <param> <value>\n", argv[0]);
        return 1;
    }
    puts("TODO: Implement cmd_set");
    // return registry_set_value(argv[1], argv[2]);
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "get", "get a parameter value", cmd_get },
    { "set", "set a parameter value", cmd_set },
    // { "list", "list parameters", cmd_list },
    // { "save", "save all parameters", cmd_save },
    // { "load", "load stored configurations", cmd_load },
    // { "dump", "dumps everything in storage", cmd_dump },
    // { "test_bytes", "dumps the bytes variable in HEX", cmd_test_bytes },
    { NULL, NULL, NULL }
};

int main(void)
{

    /* Initialize the 'registry' module */
    registry_init();

    /* Register the application configuration group */
    registry_register_config_group(&app_config_group);

    /* Initialize CBOR context for memory allocation */
    cb_ctx = cbor_memarray_init();

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
