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
#include "thread_flags.h"
#include "od.h"
#include "fmt.h"
#include "coral.h"
#include "ccm_semantics.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

/* Sync flags */
#define FLAG_SUCCESS        (0x0001)
#define FLAG_TIMEOUT        (0x0002)
#define FLAG_ERR            (0x0004)
#define FLAG_MASK           (0x0007)

/* Counts requests sent by CLI. */
static uint16_t req_count = 0;

#define CORAL_DECODE_BUF_LEN (20)
coral_element_t decode_buf[CORAL_DECODE_BUF_LEN];

static volatile thread_t *_waiter;

#define PATH_MAX_LEN    (32)
static struct {
    char menu[PATH_MAX_LEN];
    char queue[PATH_MAX_LEN];
} ccm_paths;


static int _sync(void)
{
    thread_flags_t flags = thread_flags_wait_any(FLAG_MASK);

    if (flags & FLAG_ERR) {
        return -1;
    }
    else if (flags & FLAG_TIMEOUT) {
        return -1;
    }
    else {
        return 0;
    }
}

static void _parse_machine(uint8_t *buf, size_t len)
{
    od_hex_dump(buf, len, OD_WIDTH_DEFAULT);
    coral_decode(decode_buf, CORAL_DECODE_BUF_LEN, buf, len);
    coral_print_structure(decode_buf);
    coral_element_t *e = decode_buf->children;

    while(e) {
        if (e->type == CORAL_TYPE_LINK) {
            /* look for coffee menu */
            if (!ccm_paths.menu[0] &&
                !strncmp(e->v.link.rel_type.str, CCM_MENU_TYPE,
                         e->v.link.rel_type.len)) {
                DEBUG("Found the coffee menu at: ");
                DEBUG("%.*s\n", e->v.link.target.v.as_str.len, e->v.link.target.v.as_str.str);
                memcpy(ccm_paths.menu, e->v.link.target.v.as_str.str,
                       e->v.link.target.v.as_str.len);
                ccm_paths.menu[e->v.link.target.v.as_str.len] = '\0';
            }

            /* look for coffee queue */
            if (!ccm_paths.queue[0] &&
                !strncmp(e->v.link.rel_type.str, CCM_QUEUE_TYPE,
                         e->v.link.rel_type.len)) {
                DEBUG("Found the coffee queue at: ");
                DEBUG("%.*s\n", e->v.link.target.v.as_str.len, e->v.link.target.v.as_str.str);
                memcpy(ccm_paths.queue, e->v.link.target.v.as_str.str,
                       e->v.link.target.v.as_str.len);
                ccm_paths.queue[e->v.link.target.v.as_str.len] = '\0';
            }
        }
        e = e->next;
    }
}

static void _on_discovery(unsigned req_state, coap_pkt_t *pdu,
                          sock_udp_ep_t *remote)
{
    (void)remote;
    thread_flags_t flag = FLAG_ERR;

    if (req_state == GCOAP_MEMO_RESP) {
        _parse_machine(pdu->payload, pdu->payload_len);
        flag = FLAG_SUCCESS;
    }
    else if (req_state == GCOAP_MEMO_TIMEOUT) {
        flag = FLAG_TIMEOUT;
    }

    thread_flags_set((thread_t *)_waiter, flag);
}

static size_t _send(uint8_t *buf, size_t len, char *addr_str, char *port_str,
                    gcoap_resp_handler_t resp_handler)
{
    ipv6_addr_t addr;
    size_t bytes_sent;
    sock_udp_ep_t remote;

    remote.family = AF_INET6;

    /* parse for interface */
    int iface = ipv6_addr_split_iface(addr_str);
    if (iface == -1) {
        if (gnrc_netif_numof() == 1) {
            /* assign the single interface found in gnrc_netif_numof() */
            remote.netif = (uint16_t)gnrc_netif_iter(NULL)->pid;
        }
        else {
            remote.netif = SOCK_ADDR_ANY_NETIF;
        }
    }
    else {
        if (gnrc_netif_get_by_pid(iface) == NULL) {
            puts("gcoap_cli: interface not valid");
            return 0;
        }
        remote.netif = iface;
    }

    /* parse destination address */
    if (ipv6_addr_from_str(&addr, addr_str) == NULL) {
        puts("gcoap_cli: unable to parse destination address");
        return 0;
    }
    if ((remote.netif == SOCK_ADDR_ANY_NETIF) && ipv6_addr_is_link_local(&addr)) {
        puts("gcoap_cli: must specify interface for link local target");
        return 0;
    }
    memcpy(&remote.addr.ipv6[0], &addr.u8[0], sizeof(addr.u8));

    /* parse port */
    remote.port = atoi(port_str);
    if (remote.port == 0) {
        puts("gcoap_cli: unable to parse destination port");
        return 0;
    }

    bytes_sent = gcoap_req_send(buf, len, &remote, resp_handler);
    if (bytes_sent > 0) {
        req_count++;
    }
    return bytes_sent;
}


static int _discover_resources(coap_pkt_t *pdu, uint8_t *buf, char *addr, char *port)
{
    gcoap_req_init(pdu, buf, GCOAP_PDU_BUF_SIZE, COAP_METHOD_GET, "/ccm");
    coap_hdr_set_type(pdu->hdr, COAP_TYPE_CON);
    size_t len = coap_opt_finish(pdu, COAP_OPT_FINISH_NONE);
    if (!_send(buf, len, addr, port, _on_discovery)) {
        puts("could not send discovery request");
        return -1;
    }
    _waiter = sched_active_thread;
    return _sync();
}


static void _on_menu_get(unsigned req_state, coap_pkt_t *pdu,
                         sock_udp_ep_t *remote)
{
    (void)remote;
    thread_flags_t flag = FLAG_ERR;

    if (req_state == GCOAP_MEMO_RESP) {
        puts("== Coffee Menu ==");
        printf("%.*s\n", pdu->payload_len, pdu->payload);
        flag = FLAG_SUCCESS;
    }
    else if (req_state == GCOAP_MEMO_TIMEOUT) {
        puts("Timedout while getting the menu");
        flag = FLAG_TIMEOUT;
    }

    thread_flags_set((thread_t *)_waiter, flag);
}

static int _print_menu(char *addr, char *port)
{
    coap_pkt_t pdu;
    uint8_t buf[GCOAP_PDU_BUF_SIZE];

    if (!ccm_paths.menu[0]) {
        if (_discover_resources(&pdu, buf, addr, port) < 0) {
            puts("Could not get a machine description");
            return 1;
        }
    }
    if (!ccm_paths.menu[0]) {
        puts("Could not find the coffee menu");
        return 1;
    }
    gcoap_req_init(&pdu, buf, GCOAP_PDU_BUF_SIZE, COAP_METHOD_GET,
                   ccm_paths.menu);
    coap_hdr_set_type(pdu.hdr, COAP_TYPE_CON);
    size_t len = coap_opt_finish(&pdu, COAP_OPT_FINISH_NONE);
    if (!_send(buf, len, addr, port, _on_menu_get)) {
        puts("could not send the menu request");
        return 1;
    }
    _waiter = sched_active_thread;
    return _sync();
}

static void _on_queue_get(unsigned req_state, coap_pkt_t *pdu,
                          sock_udp_ep_t *remote)
{
    (void)remote;
    thread_flags_t flag = FLAG_ERR;

    if (req_state == GCOAP_MEMO_RESP) {
        od_hex_dump(pdu->payload, pdu->payload_len, OD_WIDTH_DEFAULT);
        coral_decode(decode_buf, CORAL_DECODE_BUF_LEN, pdu->payload,
                     pdu->payload_len);
        coral_print_structure(decode_buf);
        flag = FLAG_SUCCESS;
    }
    else if (req_state == GCOAP_MEMO_TIMEOUT) {
        puts("Timed out while getting the menu");
        flag = FLAG_TIMEOUT;
    }

    thread_flags_set((thread_t *)_waiter, flag);
}

static int _print_queue(char *addr, char *port)
{
    coap_pkt_t pdu;
    uint8_t buf[GCOAP_PDU_BUF_SIZE];

    if (!ccm_paths.queue[0]) {
        if (_discover_resources(&pdu, buf, addr, port) < 0) {
            puts("Could not get a machine description");
            return 1;
        }
    }
    if (!ccm_paths.queue[0]) {
        puts("Could not find the coffees queue");
        return 1;
    }
    gcoap_req_init(&pdu, buf, GCOAP_PDU_BUF_SIZE, COAP_METHOD_GET,
                   ccm_paths.queue);
    coap_hdr_set_type(pdu.hdr, COAP_TYPE_CON);
    size_t len = coap_opt_finish(&pdu, COAP_OPT_FINISH_NONE);
    if (!_send(buf, len, addr, port, _on_queue_get)) {
        puts("could not send the queue request");
        return 1;
    }
    _waiter = sched_active_thread;
    return _sync();
}

int ccm_client_cmd(int argc, char **argv)
{
    if (argc == 1) {
        goto usage;
    }

    if (!strcmp(argv[1], "menu")) {
        return _print_menu(argv[2], argv[3]);
    }
    else if (!strcmp(argv[1], "queue")) {
        return _print_queue(argv[2], argv[3]);
    }

usage:
    printf("usage: %s <menu|queue|order> <addr> <port>\n", argv[0]);
    return 1;
}


void ccm_client_cli_init(void)
{
    coral_init();
}