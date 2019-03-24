/*
 * Copyright (C) 2019 HAW Hamburg
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
 * @brief       CIRI test application
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include <stdio.h>

#include "shell.h"
#include "shell_commands.h"
#include "net/ipv6/addr.h"
#include "ciri.h"

#define CIRI_OPTS_BUFFER_LEN (7)

static ciri_opt_t opts[CIRI_OPTS_BUFFER_LEN];

static void _print_opt(ciri_opt_t *href)
{
    switch (href->type) {
        case CIRI_OPT_SCHEME:
            printf("- scheme: %.*s", href->v.string.len, href->v.string.str);
            break;
        case CIRI_OPT_HOST_NAME:
            printf("- host name: %.*s", href->v.string.len, href->v.string.str);
            break;
        case CIRI_OPT_HOST_IP:
            printf("- host IP: ");
            ipv6_addr_print(&href->v.ip);
            break;
        case CIRI_OPT_PORT:
            printf("- port: %d", href->v.integer);
            break;
        case CIRI_OPT_PATH_TYPE:
            printf("- path type: %d", href->v.path_type);
            break;
        case CIRI_OPT_PATH:
            printf("- path: %.*s", href->v.string.len, href->v.string.str);
            break;
        case CIRI_OPT_QUERY:
            printf("- query: %.*s", href->v.string.len, href->v.string.str);
            break;
        case CIRI_OPT_FRAGMENT:
            printf("- fragment: %.*s", href->v.string.len, href->v.string.str);
            break;
        default:
            printf("unknown");
            break;
    }
}

static void print_options(ciri_opt_t *href)
{
    puts("Options");
    while (href) {
        _print_opt(href);
        puts("");
        href = href->next;
    }
}


static int _decompose_cmd(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: %s [URI]\n", argv[0]);
        return -1;
    }
    (void)_print_opt;
    (void)opts;
    //ciri_decompose(opts, CIRI_OPTS_BUFFER_LEN, argv[1], strlen(argv[1]));
    // ciri_opt_t *opt = opts;
    // while(opt) {
    //     _print_opt(opt);
    //     opt = opt->next;
    // }
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "decompose", "Decomposes a given URI into CIRI options", _decompose_cmd }
};

int main(void)
{
    ciri_opt_t scheme = { .type = CIRI_OPT_SCHEME,
                          .v.string = { "coap", sizeof("coap") } };
    ciri_opt_t host_ip = { .type = CIRI_OPT_HOST_IP, .next = NULL,
                           .v.ip = {{0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00, 0x00,
                                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                     0x00, 0x01}} };
    ciri_opt_t port = { .type = CIRI_OPT_PORT, .v.integer = 5683 };
    ciri_opt_t path1 = { .type = CIRI_OPT_PATH,
                         .v.string = { "well-known", sizeof("well-known") } };
    ciri_opt_t path2 = { .type = CIRI_OPT_PATH,
                         .v.string = { "core", sizeof("core") } };
    scheme.next = &host_ip;
    host_ip.next = &port;
    port.next = &path1;
    path1.next = &path2;

    puts("Constrained Internationalized Resource Identifiers (CIRI)\n"
         "Test application");

    print_options(&scheme);
    puts("");

    printf("- [CHECK] well formed: ");
    if (ciri_is_well_formed(&scheme) != CIRI_RET_OK) {
        puts("Fail");
    }
    else {
        puts("Success");
    }

    printf("- [CHECK] absolute: ");
    if (ciri_is_absolute(&scheme) != CIRI_RET_OK) {
        puts("No");
    }
    else {
        puts("Yes");
    }

    printf("- [CHECK] relative: ");
    if (ciri_is_relative(&scheme) != CIRI_RET_OK) {
        puts("No");
    }
    else {
        puts("Yes");
    }

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
