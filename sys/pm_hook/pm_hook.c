/*
 * Copyright (C) 2016 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_pm_layered
 * @{
 *
 * @file
 * @brief       Platform-independent power management code
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 * @}
 */

#include "pm_hook.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static clist_node_t hooks;

void pm_hook_init(void)
{
    hooks.next = NULL;
}

void pm_hook_register(pm_hook_t *hook)
{
    assert(hook);
    clist_rpush(&hooks, (clist_node_t *)hook);
}

int pm_hook_save(void)
{
    pm_hook_t *hook = (pm_hook_t *)hooks.next;

    if (!hook) {
        DEBUG("No hooks registered\n");
        return 0;
    }

    do {
        hook = (pm_hook_t *)hook->next;
        int res = hook->save(hook->ctx);
        if (res) {
            DEBUG("A save hook failed\n");
            return res;
        }
    } while(hook != (pm_hook_t *)hooks.next);

    return 0;
}

int pm_hook_restore(void)
{
    pm_hook_t *hook = (pm_hook_t *)hooks.next;

    if (!hook) {
        DEBUG("No hooks registered\n");
        return 0;
    }

    do {
        hook = (pm_hook_t *)hook->next;
        int res = hook->restore(hook->ctx);
        (void)res;
        assert(!res);
    } while(hook != (pm_hook_t *)hooks.next);

    return 0;
}
