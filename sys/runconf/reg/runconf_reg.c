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
    size_t name_len;
};

/* static variables declarations */
static clist_node_t _groups;

/* static functions declarations */
static int _cmp_group_name(clist_node_t *current, void *name);
static int _parse_key_name(const char *name, size_t name_len, const char **key);
static const runconf_reg_key_t *_find_key_in_group(runconf_reg_group_t *group,
                                                   const char *name,
                                                   size_t name_len);

/* static functions definitions */
static int _cmp_group_name(clist_node_t *current, void *name)
{
    runconf_reg_group_t *group = NODE_TO_GROUP(current);
    struct group_name *_name = (struct group_name *)name;
    size_t group_len = strlen(group->name);

    if (_name->name_len == group_len) {
        return !strncmp(group->name, _name->name, group_len);
    }
    return 0;
}

static int _parse_key_name(const char *name, size_t name_len, const char **key)
{
    const char *pos = name;
    const char *end = name + name_len;

    *key = NULL;

    while (pos < end) {
        if (*pos == RUNCONF_REG_KEY_SEPARATOR[0]) {
            *key = pos;
        }
        pos++;
    }

    if (*key && *key + 1 < end) {
        (*key)++;
        return 0;
    }

    return -1;
}

static const runconf_reg_key_t *_find_key_in_group(runconf_reg_group_t *group,
                                                   const char *name,
                                                   size_t name_len)
{
    const runconf_reg_key_t *pos = group->keys;

    while (pos < &group->keys[group->keys_numof]) {
        if (name_len == strlen(pos->name) &&
            !strncmp(pos->name, name, name_len)) {
            return pos;
        }
        pos++;
    }
    return NULL;
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

runconf_reg_group_t *runconf_reg_get_group(const char *name, size_t name_len)
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

runconf_reg_group_t *runconf_reg_get_key(const char *name, size_t name_len,
                                         const runconf_reg_key_t **out)
{
    assert(name);    
    const char *key;
    if (_parse_key_name(name, name_len, &key)) {
        DEBUG("[%s] Could not parse key name\n", __func__);
        return NULL;
    }

    runconf_reg_group_t *group = runconf_reg_get_group(name, key - 1 - name);
    if (!group) {
        DEBUG("[%s] Could not find the configuration group %.*s. L%d\n", __func__,
              key - 1 - name, name, key - 1 - name);
        return NULL;
    }

    *out = _find_key_in_group(group, key, name + name_len - key);

    return *out ? group : NULL;
}
