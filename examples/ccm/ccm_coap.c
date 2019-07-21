/*
 * Copyright (c) 2015-2017 Ken Bannister. All rights reserved.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       gcoap CLI support
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 *
 * @}
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net/gcoap.h"
#include "od.h"
#include "fmt.h"
#include "coral.h"
#include "ccm.h"
#include "ccm_internal.h"
#include "ccm_semantics.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#define COAP_FORMAT_HTCPC   (65502)

static ssize_t _ccm_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx);
static ssize_t _ccm_menu_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx);

ssize_t _get_coral_representation(uint8_t *buf, size_t len);
static ssize_t _get_machine_coral(uint8_t *buf, size_t len);

#define CCM_QUEUE_SIZE  (10)

static ccm_t coffee_machine;
static ccm_queue_item_t coffee_queue[CCM_QUEUE_SIZE];

#define CCM_RESOURCE_BASE   "/ccm"
#define CCM_RESOURCE_MENU   CCM_RESOURCE_BASE "/m"

static const char *_ccm_resource_menu = CCM_RESOURCE_MENU;

/* CoAP resources. Must be sorted by path (ASCII order). */
static const coap_resource_t _resources[] = {
    {
        .path = CCM_RESOURCE_BASE,
        .methods = COAP_GET,
        .handler = _ccm_handler,
    },
    {
        .path = CCM_RESOURCE_MENU,
        .methods = COAP_GET,
        .handler = _ccm_menu_handler,
    },
};

static gcoap_listener_t _listener = {
    &_resources[0],
    sizeof(_resources) / sizeof(_resources[0]),
    NULL
};


static ssize_t _get_machine_coral(uint8_t *buf, size_t len)
{
    coral_element_t coral_doc;
    coral_create_document(&coral_doc);

    /* create a CoRAL link of count */
    coral_element_t coffee_count_resource;
    coral_link_target_t coffee_count_value;
    coral_literal_int(&coffee_count_value, (int)ccm_queue_get_count(&coffee_machine));
    coral_create_link(&coffee_count_resource, "count", &coffee_count_value);
    coral_append_element(&coral_doc, &coffee_count_resource);

    coral_print_structure(&coral_doc);
    size_t used = coral_encode(&coral_doc, buf, len);
    od_hex_dump(buf, used, OD_WIDTH_DEFAULT);

    return used;
}

/**
 * brief Encodes the menu in the format "example/htcpc"
 */
static size_t _encode_menu(uint8_t *buf, size_t len)
{
    (void)len; // TODO should check
    char *_buf = (char *)buf;
    size_t pos = 0;

    _buf[pos++] = '{';

    /* add milks */
    strcpy(&_buf[pos], "\"milk\":[");
    pos += sizeof("\"milk\":[") - 1;

    for (unsigned i = 0; i < MILK_NUMOF; i++) {
        if (i){
            _buf[pos++] = ',';
        }
        _buf[pos++] = '"';
        strcpy(&_buf[pos], _menu_milk_str[i]);
        pos += _menu_milk_str_size[i];
        _buf[pos++] = '"';
    }

    /* add syrups */
    strcpy(&_buf[pos], "],\"syrup\":[");
    pos += sizeof("],\"syrup\":[") - 1;

    for (unsigned i = 0; i < SYRUP_NUMOF; i++) {
        if (i){
            _buf[pos++] = ',';
        }
        _buf[pos++] = '"';
        strcpy(&_buf[pos], _menu_syrup_str[i]);
        pos += _menu_syrup_str_size[i];
        _buf[pos++] = '"';
    }

    /* add alcohols */
    strcpy(&_buf[pos], "],\"alcohol\":[");
    pos += sizeof("],\"alcohol\":[") - 1;

    for (unsigned i = 0; i < ALCOHOL_NUMOF; i++) {
        if (i){
            _buf[pos++] = ',';
        }
        _buf[pos++] = '"';
        strcpy(&_buf[pos], _menu_alcohol_str[i]);
        pos += _menu_alcohol_str_size[i];
        _buf[pos++] = '"';
    }

    _buf[pos++] = ']';
    _buf[pos++] = '}';
    return pos;
}

static ssize_t _ccm_menu_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    DEBUG("The menu is being requested\n");
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    // coap_opt_add_format(pdu, COAP_FORMAT_HTCPC);
    coap_opt_add_format(pdu, COAP_FORMAT_JSON);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
    resp_len += _encode_menu(pdu->payload, pdu->payload_len);
    return resp_len;
}

ssize_t _get_coral_representation(uint8_t *buf, size_t len)
{
    coral_element_t coral_doc;
    coral_create_document(&coral_doc);

    /* create link for menu resource */
    coral_element_t coffee_menu_resource;
    coral_link_target_t coffee_menu_value;
    coral_literal_string(&coffee_menu_value, _ccm_resource_menu);
    coral_create_link(&coffee_menu_resource, _ccm_menu_type, &coffee_menu_value);
    coral_append_element(&coral_doc, &coffee_menu_resource);

    /*  */

    coral_print_structure(&coral_doc);
    ssize_t used = coral_encode(&coral_doc, buf, len);
    od_hex_dump(buf, used, OD_WIDTH_DEFAULT);
    return used;
}

static ssize_t _ccm_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;

    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_CBOR);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

    ssize_t rep_len = _get_coral_representation(pdu->payload, pdu->payload_len);
    (void)_get_machine_coral;
    if (rep_len < 0) {
        DEBUG("Error getting coral representation of the resources\n");
        return 0;
    }

    resp_len += (size_t)rep_len;
    return resp_len;
}


void ccm_coap_init(void)
{
    gcoap_register_listener(&_listener);
    ccm_init(&coffee_machine, coffee_queue,
             sizeof(coffee_queue) / sizeof(coffee_queue[0]));
}
