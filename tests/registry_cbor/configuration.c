#include <stdio.h>
#include <string.h>

#include "cn-cbor/cn-cbor.h"
#include "registry/registry.h"

/* Size of the test_bytes configuration parameter */
#ifndef BYTES_LENGTH
#define BYTES_LENGTH    16
#endif

/* These are the 'configuration parameters' */
static const char *test_opt1_str = "opt1";
static const char *test_opt2_str = "opt2";
static const char *test_bytes_str = "bytes";
static int64_t test_opt1 = 0;
static int8_t test_opt2 = 1;
static unsigned char test_bytes[BYTES_LENGTH] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                                 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                                 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                                 0xAA};

static int set_handler(cn_cbor *cb, void *ctx);
static cn_cbor *get_handler(cn_cbor *cb, cn_cbor_context *cb_ctx, void *ctx);

/* This is the configuration group descriptor for the parameters of the
 * application. It has an unique name and handler functions for the different
 * acctions required. */
registry_config_group_t app_config_group = {
    .name = "app",
    .get = get_handler,
    .set = set_handler,
};

static cn_cbor *get_handler(cn_cbor *cb, cn_cbor_context *cb_ctx, void *ctx)
{
    (void)ctx;
    cn_cbor *config = cn_cbor_map_create(cb_ctx, NULL);

    if (cb) {
        cn_cbor *current = cb->first_child;
        while (current) {
            if (!strncmp(current->v.str, test_opt1_str, current->length)) {
                cn_cbor *val = cn_cbor_int_create(test_opt1, cb_ctx, NULL);
                cn_cbor_mapput_string(config, test_opt1_str, val, cb_ctx, NULL);
            }
            else if (!strncmp(current->v.str, test_opt2_str, current->length)) {
                cn_cbor *val = cn_cbor_int_create(test_opt2, cb_ctx, NULL);
                cn_cbor_mapput_string(config, test_opt2_str, val, cb_ctx, NULL);
            }
            else if (!strncmp(current->v.str, test_bytes_str, current->length)) {
                cn_cbor *val = cn_cbor_data_create(test_bytes, BYTES_LENGTH, cb_ctx, NULL);
                cn_cbor_mapput_string(config, test_bytes_str, val, cb_ctx, NULL);
            }
            current = current->next;
        }
    }
    else {
        cn_cbor *val = cn_cbor_int_create(test_opt1, cb_ctx, NULL);
        cn_cbor_mapput_string(config, test_opt1_str, val, cb_ctx, NULL);
        val = cn_cbor_int_create(test_opt2, cb_ctx, NULL);
        cn_cbor_mapput_string(config, test_opt2_str, val, cb_ctx, NULL);
        val = cn_cbor_data_create(test_bytes, BYTES_LENGTH, cb_ctx, NULL);
        cn_cbor_mapput_string(config, test_bytes_str, val, cb_ctx, NULL);
    }
    return config;
}

static int set_handler(cn_cbor *cb, void *ctx)
{
    (void)ctx;
    (void)cb;
    // if (!strcmp(test_opt1_str, name)) {
    //     test_opt1 = cb->v.sint;
    // }
    // else if (!strcmp(test_opt2_str, name)) {
    //     test_opt2 = cb->v.sint;
    // }
    // else if (!strcmp(test_bytes_str, name)) {
    //     memcpy(test_bytes, cb->v.bytes, BYTES_LENGTH);
    // }
    // else {
    //     return -1;
    // }
    return 0;
}

