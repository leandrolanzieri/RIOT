#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "runconf/runconf_reg.h"

/* these are the configurable parameters */
int threshold = -5;
unsigned int delay = 30;
unsigned int lorawan_dr = 5;
uint8_t lorawan_appkey[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
uint8_t lorawan_appeui[] = {0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB};

static int _get_handler(char *path, unsigned path_len, runconf_reg_key_t *key,
                        runconf_reg_value_t *val, void *ctx)
{
    printf("Getting parameter %s\n", key->name);
    printf("With path: %*.s\n", path_len, path);
    (void)val;
    (void)ctx;
    return 0;
}

static int _set_handler(char *path, unsigned path_len, runconf_reg_key_t *key,
                        runconf_reg_value_t *val, void *ctx)
{
    printf("Setting parameter %s\n", key->name);
    printf("With path: %*.s\n", path_len, path);
    (void)val;
    (void)ctx;
    return 0;
}

static const runconf_reg_key_t _params[] = {
    {
        .name = "appkey",
        .type = RUNCONF_REG_TYPE_BYTES,
    },
    {
        .name = "appeui",
        .type = RUNCONF_REG_TYPE_BYTES,
    },
    {
        .name = "deveui",
        .type = RUNCONF_REG_TYPE_BYTES,
    }
};

static runconf_reg_group_t _group = {
    .name = "net/lorawan",
    .keys = _params,
    .keys_numof = (sizeof(_params) / sizeof(*_params)),
    .get = _get_handler,
    .set = _set_handler
};

int main(void)
{
    runconf_reg_add_group(&_group);

    return 0;
}
