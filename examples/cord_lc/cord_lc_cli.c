/*
 * Copyright (C) 2018 HAW Hamburg
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
 * @brief       Shell command for the cord_lc module
 *
 * @author      Aiman Ismail <muhammadaimanbin.ismail@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>

#include "net/cord/lc.h"
#include "net/gcoap.h"
#include "net/sock/util.h"
#include "net/cord/config.h"

#define CORD_SERVER_PORT (5683)
#define FILTER_BUF_SIZE  (64)
#define RESULT_BUF_SIZE  (1024)

static cord_lc_rd_t rd;
static sock_udp_ep_t remote;
static char rdbuf[2 * NANOCOAP_URI_MAX];
static unsigned rd_initialized = 0;

static int _make_sock_ep(sock_udp_ep_t *ep, const char *addr)
{
    ep->port = 0;
    if (sock_udp_str2ep(ep, addr) < 0) {
        return -1;
    }
    ep->family  = AF_INET6;
    ep->netif   = SOCK_ADDR_ANY_NETIF;
    if (ep->port == 0) {
        ep->port = CORD_SERVER_PORT;
    }
    return 0;
}

/**
 * Parses main arguments for filters.
 * Returns number of parsed filters
 */
static void _parse_filters(clif_param_t *filters, size_t filter_count,
                              char **argv)
{
    for (unsigned i = 0; i < filter_count; i++) {
        clif_param_t *f = &filters[i];
        f->key = argv[i];
        char *key_end = memchr(argv[i], '=', strlen(argv[i]));
        if (!key_end) {
		f->key_len = strlen(f->key);
		f->value = NULL;
		f->value_len = 0;
		continue;
	}

        f->key_len = key_end - f->key;

        char *value_start = key_end + 1;
        char *value_end = memchr(value_start, '\0', sizeof(argv[i]) - (f->key_len + 1));
        f->value = value_start;
        f->value_len = value_end - value_start;
    }
}

static void _print_lookup_result(struct cord_lc_result *res)
{
    printf("Found resource/endpoint\n");
    printf("Target: %.*s\n", res->link.target_len, res->link.target);
    for (unsigned i = 0; i < res->link.params_len; i++) {
        clif_param_t *p = &(res->link.params[i]);
        printf("Key: %.*s\nValue: %.*s\n", p->key_len, p->key, p->value_len, p->value);
    }
}

static void _print_usage(void)
{
    puts("usage: cord_lc <server_addr> "
         "{ resource | endpoint | raw { resource | endpoint } "
         "[key=value]");
    puts("example: cord_lc [2001:db8:3::dead:beef]:5683 raw resource count=1 page=2");
}

int cord_lc_cli_cmd(int argc, char **argv)
{
    int ret;
    char bufpool[1024];
    memset(bufpool, 0, sizeof(bufpool));

    if (argc < 3) {
        _print_usage();
        return -1;
    }

    if (!rd_initialized) {
        ret = _make_sock_ep(&remote, argv[1]);
        if (ret < 0) {
            printf("error: unable to parse address\n");
            return -1;
        }
        puts("Performing lookup now, this may take a short while...");
        ret = cord_lc_rd_init(&rd, &remote, rdbuf, sizeof(rdbuf));
        if (ret < 0) {
            printf("error initializing RD server %d", ret);
            return -1;
        }
        rd_initialized = 1;
    }

    unsigned filter_start;
    if (!strcmp(argv[2], "raw")) {
        filter_start = 4;
    }
    else {
        filter_start = 3;
    }

    /* parse filters */
    size_t filter_count = argc - filter_start;
    clif_param_t filter_array[filter_count];
    if (filter_count > 0) {
        _parse_filters(filter_array, filter_count, &argv[filter_start]);
    }
    cord_lc_filter_t filters = {
        .array = filter_array,
        .len = filter_count,
        .next = NULL,
    };

    int retval = 0;
    if (!strcmp(argv[2], "raw")) {
        if (!strcmp(argv[3], "resource")) {
            retval = cord_lc_raw(&rd, COAP_FORMAT_LINK, CORD_LC_RES, &filters,
                                 bufpool, sizeof(bufpool));
        }
        else if (!strcmp(argv[3], "endpoint")) {
            retval = cord_lc_raw(&rd, COAP_FORMAT_LINK, CORD_LC_EP, &filters,
                                 bufpool, sizeof(bufpool));
        }
        else {
            _print_usage();
            return -1;
        }
        if (retval < 0) {
            printf("Error during lookup %d\n", retval);
            return -1;
        }
        printf("Lookup result:\n%.*s\n", retval, bufpool);
    }
    else if (!strcmp(argv[2], "resource")) {
        cord_lc_res_t resource;
	clif_param_t params[5];
	resource.params = params;
	resource.max_params = sizeof(params) / sizeof(clif_param_t);
        retval = cord_lc_res(&rd, &resource, &filters, bufpool,
                             sizeof(bufpool));
        if (retval < 0) {
            printf("Error during lookup %d\n", retval);
            return -1;
        }
        _print_lookup_result(&resource);
    }
    else if (!strcmp(argv[2], "endpoint")) {
        cord_lc_ep_t endpoint;
	clif_param_t params[5];
	endpoint.params = params;
	endpoint.max_params = sizeof(params) / sizeof(clif_param_t);
        retval = cord_lc_ep(&rd, &endpoint, &filters, bufpool,
                             sizeof(bufpool));
        if (retval < 0) {
            printf("Error during lookup %d\n", retval);
            return -1;
        }
        _print_lookup_result(&endpoint);
    }
    else {
        _print_usage();
        return -1;
    }
    return 0;
}
