/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_runconf_reg RunConf Registry
 * @ingroup     sys
 * @brief       Global registry for runtime configuration management (RunConf)
 *
 * @{
 *
 * @file
 * @brief       RunConf Registry interface definition
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */

#ifndef RUNCONF_RUNCONF_REG_H
#define RUNCONF_RUNCON_REG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include "clist.h"

#ifndef RUNCONF_REG_KEY_SEPARATOR
#define RUNCONF_REG_KEY_SEPARATOR   "/"
#endif

typedef union runconf_reg_data {
    int sint;
    unsigned int uint;
    char *s;
    struct {
        uint8_t *p;
        unsigned len;
    } bytes;
    bool b;
#ifdef RUNCONF_REG_USE_FLOAT
    float f;
#endif /* RUNCONF_REG_USE_FLOAT */
} runconf_reg_data_t;

typedef enum runconf_reg_data_type {
    RUNCONF_REG_TYPE_SIGNED_INT = 0,
    RUNCONF_REG_TYPE_UNSIGNED_INT,
    RUNCONF_REG_TYPE_STRING,
    RUNCONF_REG_TYPE_BYTES,
    RUNCONF_REG_TYPE_BOOL,
    RUNCONF_REG_TYPE_SUBTREE
#ifdef RUNCONF_REG_USE_FLOAT
    RUNCONF_REG_TYPE_FLOAT,
#endif /* RUNCONF_REG_USE_FLOAT */
} runconf_reg_data_type_t;

typedef struct runconf_reg_value {
    runconf_reg_data_t data;
    runconf_reg_data_type_t type;
} runconf_reg_value_t;

typedef uint8_t runconf_reg_key_flags_t;

typedef struct runconf_reg_key {
    const char *name;
    runconf_reg_data_type_t type;
    runconf_reg_key_flags_t flags;
} runconf_reg_key_t;

typedef int (*runconf_reg_get_handler_t)(char *path, unsigned path_len,
                                         runconf_reg_key_t *key,
                                         runconf_reg_value_t *val, void *ctx);

typedef runconf_reg_get_handler_t runconf_reg_set_handler_t;

typedef struct runconf_reg_group {
    clist_node_t node;
    char *name;
    const runconf_reg_key_t *keys;
    unsigned keys_numof;
    runconf_reg_get_handler_t get;
    runconf_reg_set_handler_t set;
    void *ctx;
} runconf_reg_group_t;

void runconf_reg_init(void);

void runconf_reg_add_group(runconf_reg_group_t *group);

runconf_reg_group_t *runconf_reg_get_group(const char *name, size_t name_len);

runconf_reg_group_t *runconf_reg_get_key(const char *name, size_t name_len,
                                         const runconf_reg_key_t **out);

int runconf_reg_set_value(int name_argc, char **name_argv,
                          runconf_reg_group_t *group, runconf_reg_key_t *key,
                          runconf_reg_value_t *val);

int runconf_reg_set_value_from_str(char *name, size_t name_len, char *val,
                                   size_t val_len);

#ifdef __cplusplus
}
#endif

#endif /* RUNCONF_RUNCONF_REG_H */