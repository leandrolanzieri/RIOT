/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_runconf_reg
 * @{
 *
 * @file
 * @brief       RunConf Registry
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */

#include <assert.h>
#include <string.h>
#include "runconf/runconf_reg.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#define NODE_TO_GROUP(node)   ((runconf_reg_group_t *)node)
#define GROUP_TO_NODE(group)  ((clist_node_t *)group)

struct group_name {
    const char *name;
    ssize_t name_len;
};

/* static variables declarations */
static clist_node_t _groups;

/* static functions declarations */
static int _cmp_group_name(clist_node_t *current, void *name);

/* static functions definitions */
static int _cmp_group_name(clist_node_t *current, void *name)
{
    runconf_reg_group_t *group = NODE_TO_GROUP(current);
    struct group_name *_name = (struct group_name *)name;

    if (_name->name_len > 0 && strlen(group->name) != (size_t)_name->name_len) {
        return false;
    }
    else {
        return !strcmp(group->name, _name->name);
    }
}

void runconf_reg_init(void)
{
    _groups.next = NULL;
}

void runconf_reg_add_group(runconf_reg_group_t *group)
{
    assert(group);
    clist_node_t *node = GROUP_TO_NODE(group);
    clist_rpush(&_groups, node);
}

runconf_reg_group_t *runconf_reg_get_group(const char *name, ssize_t name_len)
{
    clist_node_t *node;
    runconf_reg_group_t *group =  NULL;
    struct group_name _name = { name, name_len };
    node = clist_foreach(&_groups, _cmp_group_name, &_name);

    if (node != NULL) {
        group = NODE_TO_GROUP(node);
    }

    return group;
}

int runconf_reg_parse_key(char *name, int *name_argc, char **name_argv,
                          unsigned name_argv_len)
{
    unsigned i = 0;
    char *res = strtok(name, RUNCONF_REG_KEY_SEPARATOR);

    while (res && i < name_argv_len) {
        name_argv[i++] = res;
        res = strtok(NULL, RUNCONF_REG_KEY_SEPARATOR);
    }
    *name_argc = i;

    if (!res) {
        return 0;
    }
    return -1;
}

runconf_reg_group_t *runconf_reg_get_key(const char *name, ssize_t name_len,
                                         runconf_reg_key_t *out)
{
    (void)name;
    (void)name_len;
    (void)out;
    return NULL;
}
