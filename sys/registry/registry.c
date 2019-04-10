#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "clist.h"
#include "registry/registry.h"
#include "kernel_defines.h"
#include "assert.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

static registry_config_group_t *_lookup_group(const char *name, size_t name_len);

clist_node_t registry_config_groups;

static registry_config_group_t *_lookup_group(const char *name, size_t name_len)
{
    registry_config_group_t *group = (registry_config_group_t *)registry_config_groups.next;
    registry_config_group_t *res = NULL;

    if (!group) {
        goto out;
    }

    do {
        DEBUG("Comparing %s to %.*s\n", group->name, name_len, name);
        if (!strncmp(group->name, name, name_len)) {
            res = group;
            goto out;
        }
    } while (&group->node != registry_config_groups.next);

out:
    return res;
}

void registry_init(void)
{
    registry_config_groups.next = NULL;
}

void registry_register(registry_config_group_t *handler)
{
    assert(handler != NULL);
    clist_rpush(&registry_config_groups, &(handler->node));
}

static int _call_commit(clist_node_t *current, void *res)
{
    assert(current != NULL);
    int _res = *(int *)res;
    registry_config_group_t *group = container_of(current, registry_config_group_t, node);
    if (group->commit) {
        _res = group->commit(group->ctx);
        if (!*(int *)res) {
            *(int *)res = _res;
        }
    }
    return 0;
}

int registry_commit(char *name)
{
    int rc = 0;

    if (name) {
        registry_config_group_t *group;

        group = _lookup_group(name, strlen(name));
        if (!group) {
            return -EINVAL;
        }
        if (group->commit) {
            return group->commit(group->ctx);
        }
        else {
            return 0;
        }
    }
    else {
        clist_foreach(&registry_config_groups, _call_commit,
                      (void *)(&rc));
        return rc;
    }
}

void registry_register_config_group(registry_config_group_t *group)
{
    assert(group);
    clist_rpush(&registry_config_groups, &(group->node));
}

int registry_set_config(cn_cbor *cb)
{
    assert(cb && cb->type == CN_CBOR_MAP);
    cn_cbor *current = cb->first_child;

    while (current && current->type == CN_CBOR_TEXT) {
        registry_config_group_t *group = _lookup_group(current->v.str, current->length);
        if (!group || !group->set) {
            DEBUG("[%s] Could not find group or has no 'set' handler\n", __func__);
            continue;
        }
        current = current->next;
        group->set(current, group->ctx);
        current = current->next;
    }
    return 0;
}

cn_cbor *registry_get_config(cn_cbor *cb, cn_cbor_context *cb_ctx)
{
    assert(cb_ctx);
    registry_config_group_t *group; 
    cn_cbor *root = cn_cbor_map_create(cb_ctx, NULL);

    if (!cb || !cb->length) {
        /* should return everything */
        group = (registry_config_group_t *)registry_config_groups.next;

        if (!group) {
            return NULL;
        }

        do {
            group = (registry_config_group_t *)group->node.next;
            if (group->get) {
                cn_cbor *config = group->get(NULL, cb_ctx, group->ctx);
                if (config) {
                    cn_cbor_mapput_string(root, group->name, config, cb_ctx, NULL);
                }
            }
        } while (&group->node != registry_config_groups.next);
        return root;
    }
    else {
        /* should return only the groups that match the keys */
        cn_cbor *current = cb->first_child;
        while (current) {
            DEBUG("Looking for: %.*s\n", current->length, current->v.str);
            group = _lookup_group(current->v.str, current->length);
            current = current->next;
            if (!group || !group->get) {
                DEBUG("[%s] Could not find group or has no 'get' hander\n", __func__);
            }
            else {
                cn_cbor *config;
                if (current->length) {
                    DEBUG("Map not empty, get some parameters\n");
                    /* if particular parameters are asked for pass the map */
                    config = group->get(current, cb_ctx, group->ctx);
                }
                else {
                    DEBUG("Map empty, get all parameters");
                    config = group->get(NULL, cb_ctx, group->ctx);
                }
                if (config) {
                    cn_cbor_mapput_string(root, group->name, config, cb_ctx, NULL);
                }
            }
            current = current->next;
        }
        return root;
    }
}
