#include "clist.h"

typedef int (*pm_hook_cb_t)(void *ctx);

typedef struct {
    clist_node_t *next;
    pm_hook_cb_t save;
    pm_hook_cb_t restore;
    void *ctx;
} pm_hook_t;

void pm_hook_init(void);

void pm_hook_register(pm_hook_t *hook);

int pm_hook_save(void);

int pm_hook_restore(void);
