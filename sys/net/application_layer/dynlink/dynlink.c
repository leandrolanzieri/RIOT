/*
 * Copyright (c) 2015-2017 Ken Bannister. All rights reserved.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gcoap
 * @{
 *
 * @file
 * @brief       GNRC's implementation of CoAP protocol
 *
 * Runs a thread (_pid) to manage request/response messaging.
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 */

#include <errno.h>
#include <stdint.h>
#include <stdatomic.h>
#include <string.h>

#include "assert.h"
#include "net/dynlink.h"
#include "net/gcoap.h"
#include "net/sock/util.h"
#include "fmt.h"
#include "clif.h"
#include "mutex.h"
#include "random.h"
#include "thread.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#define DYNLINK_BIND_TABLE_ENCODING "<" CONFIG_DYNLINK_BIND_TABLE_PATH ">"\
                                    ";rt=\"" DYNLINK_BIND_TABLE_RT "\"" \
                                    ";ct=40"

static ssize_t _coap_handler(coap_pkt_t *pkt, uint8_t *buf, size_t len, void *ctx);
static void _table_clean(void);
static int _table_add_binding(clif_t *link_f, char *link, size_t link_len);
static int _set_bind_type(dynlink_condition_t *cond, const char *bind, size_t bind_len);
static void _print_bindings(void);
static clif_attr_t *_get_attr_by_key(clif_t *link, const char *key);

static dynlink_condition_t _conditions[CONFIG_DYNLINK_COND_MAX];
static coap_resource_t _bind_table = {
    .path = CONFIG_DYNLINK_BIND_TABLE_PATH,
    .methods = COAP_GET | COAP_PUT,
    .handler = _coap_handler
};

static ssize_t _link_encoder(const coap_resource_t *resource, char *buf,
                             size_t maxlen, coap_link_encoder_ctx_t *context)
{
    (void) resource;
    (void) context;
    size_t enc_len = strlen(DYNLINK_BIND_TABLE_ENCODING);

    if (maxlen > enc_len) {
        memcpy(buf, DYNLINK_BIND_TABLE_ENCODING, enc_len);
        return enc_len;
    }

    return 0;
}

static ssize_t _coap_handler(coap_pkt_t *pkt, uint8_t *buf, size_t len, void *ctx)
{
    (void)pkt;
    (void)buf;
    (void)len;
    (void)ctx;
    // unsigned method_flag = coap_method2flag(coap_get_code_detail(pkt));

    // switch (method_flag) {
    //     case COAP_PUT:
    //         _table_clean();
    //         clif_t clif;
    //         clif_attr_t params[CONFIG_DYNLINK_CLIF_ATTRS_MAX];
    //         char *pos = (char *)pkt->payload;
    //         do {
    //             ssize_t res = clif_decode_link(&clif, params,
    //                                            CONFIG_DYNLINK_CLIF_ATTRS_MAX, pos,
    //                                            pkt->payload_len - (pos - (char *)pkt->payload));
    //             if (res < 0) {
    //                 break;
    //             }
    //             char *link_raw;
    //             res = clif_get_link(pos, pkt->payload_len - (pos - (char *)pkt->payload),
    //                                 &link_raw);
    //             if (_table_add_binding(&clif, link_raw, res) < 0) {
    //                 DEBUG("[Dynlink] Error adding binding\n");
    //                 _table_clean();
    //             }
    //             pos += res;
    //         } while (pos < (char *)(pkt->payload + pkt->payload_len));
    //         break;
    //     case COAP_GET:
    //         // gcoap_resp_init(pkt, buf, len, COAP_CODE_CONTENT);
    //         // coap_opt_add_format(pkt, COAP_FORMAT_LINK);
    //         _print_bindings();
    //         break;
    // }
    (void) _table_add_binding;
    (void) _print_bindings;
    return 0;
}

static void _print_bindings(void)
{
    for (unsigned i = 0; i < CONFIG_DYNLINK_COND_MAX; i++) {
        puts("-- Condition --");
        printf("> Link: %.*s\n", _conditions[i].clif_len, _conditions[i].clif);
        printf("> Bind: %d\n", _conditions[i].bind);
    }
}

static int _table_add_binding(clif_t *link_f, char *link, size_t link_len)
{
    /* find free binding entry */
    dynlink_condition_t *cond = NULL;
    for (unsigned i = 0; i < CONFIG_DYNLINK_COND_MAX; i++) {
        if (_conditions[i].bind == DYNLINK_NONE) {
            cond = &_conditions[i];
        }
    }
    if (!cond) {
        DEBUG("[Dynlink] No more space in binding table\n");
        return -1;
    }

    /* find the local resource */
    clif_attr_t *p = _get_attr_by_key(link_f, "anchor");
    const coap_resource_t *res;
    gcoap_listener_t *lis;
    if (!p) {
        DEBUG("[Dynlink] No anchor specified\n");
        return -1;
    }
    if (gcoap_find_resource_by_path(p->value, p->value_len, &res, &lis) != 0) {
        DEBUG("[Dynlink] No resource found\n");
        return -1;
    }
    cond->coap_res = res;

    /* check relation type */
    p = _get_attr_by_key(link_f, "rel");
    if (!p || p->value_len != sizeof(DYNLINK_BINDING_RT) - 1 ||
        strncmp(p->value, DYNLINK_BINDING_RT, p->value_len)) {
        DEBUG("[Dynlink] Wrong relation type for binding\n");
        return -1;
    }

    /* copy link */
    if (link_len >= DYNLINK_TARGET_MAX) {
        DEBUG("[Dynlink] Not enough space to save link\n");
        return -1;
    }
    memcpy(cond->clif, link, link_len);
    cond->clif_len = link_len;

    /* set binding type */
    p = _get_attr_by_key(link_f, "bind");
    if (!p) {
        DEBUG("[Dynlink] No binding specified\n");
        return -1;
    }
    if (_set_bind_type(cond, p->value, p->value_len) < 0) {
        DEBUG("[Dynlink] Invalid binding specified\n");
        return -1;
    }

    /* look for conditions */
    p = _get_attr_by_key(link_f, "lt");
    if (p) {
        cond->lt = scn_u32_dec(p->value, p->value_len);
    }

    p = _get_attr_by_key(link_f, "gt");
    if (p) {
        cond->gt = scn_u32_dec(p->value, p->value_len);
    }

    return 0;
}

static int _set_bind_type(dynlink_condition_t *cond, const char *bind, size_t bind_len)
{
    assert(cond && bind);
    if (!strncmp(bind, DYNLINK_BIND_OBS, bind_len)) {
        cond->bind = DYNLINK_OBS;
        return 0;
    }
    if (!strncmp(bind, DYNLINK_BIND_POLL, bind_len)) {
        cond->bind = DYNLINK_POLL;
        return 0;
    }
    if (!strncmp(bind, DYNLINK_BIND_PUSH, bind_len)) {
        cond->bind = DYNLINK_PUSH;
        return 0;
    }
    cond->bind = DYNLINK_NONE;
    return -1;
}

static clif_attr_t *_get_attr_by_key(clif_t *link, const char *key)
{
    size_t len = strlen(key);
    for (unsigned i = 0; i < link->attrs_len; i++) {

        if (len == link->attrs[i].key_len &&
            !strncmp(link->attrs[i].key, key, link->attrs[i].key_len)) {
            return &link->attrs[i];
        }
    }
    return NULL;
}

static void _table_clean(void)
{
    for (unsigned i = 0; i < sizeof(_conditions) / sizeof(_conditions[0]); i++) {
        _conditions[i].bind = DYNLINK_NONE;
    }
}

static gcoap_listener_t _listener = {
    &_bind_table,
    1,
    _link_encoder,
    NULL
};

int dynlink_init(void) {
    gcoap_register_listener(&_listener);
    _table_clean();
    return 0;
}

/** @} */
