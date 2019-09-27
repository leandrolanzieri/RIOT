/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test application for SARA-R410M modules
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "shell.h"
#include "shell_commands.h"
#include "fmt.h"
#include "board.h"
#include "xtimer.h"

#include "sara_r410m.h"
#include "sara_r410m_params.h"

static sara_r410m_t sara_dev;

static int _socket_cmd(int argc, char **argv)
{
    sara_r410m_socket_t sock;

    if (argc < 2) {
        printf("usage: %s <create|send|close>\n", argv[0]);
        return 1;
    }

    if (!strcmp(argv[1], "create")) {
        if (argc < 4) {
            printf("usage: %s create <udp|tcp> <port>\n", argv[0]);
            return 1;
        }
        sock.type = strcmp(argv[2], "tcp") ? SOCK_UDP : SOCK_TCP;

        uint16_t port = atoi(argv[3]);
        if (sara_r410m_socket_create(&sara_dev, &sock, port) != SARA_R410M_OK) {
            puts("Error: could not create socket");
            return 1;
        }
        puts("Socket created:");
        printf("- ID: %d\n- Type: %s", sock.id, sock.type == SOCK_UDP ? "UDP" : "TCP");
        return 0;
    }

    if (!strcmp(argv[1], "send")) {
        if (argc < 7) {
            printf("usage: %s send <socket ID> <udp|tcp> <addr> <port> <payload>\n", argv[0]);
            return 1;
        }
        sock.id = atoi(argv[2]);
        sock.type = strcmp(argv[3], "tcp") ? SOCK_UDP : SOCK_TCP;
        uint16_t port = atoi(argv[5]);
        if (sara_r410m_socket_send(&sara_dev, &sock, argv[4], port, argv[6],
                                   strlen(argv[6])) != SARA_R410M_OK) {
            puts("Error: could not send payload");
            return 1;
        }
        puts("Payload sent!");
        return 0;
    }

    return 1;
}

static int _sys_cmd(int argc, char **argv)
{
    if (argc != 2) {
        printf("usage: %s <register>\n", argv[0]);
        return 1;
    }

    if (!strcmp(argv[1], "register")) {
        if (sara_r410m_register(&sara_dev) != SARA_R410M_OK) {
            puts("Error: Could not register to network");
            return 1;
        }
        return 0;
    }
    return 1;
}

#define ATT_IP      "40.68.172.187"
#define ATT_PORT    8891
#define ATT_ID      "pIk77bl2REiBvCpzhvK2DffA"
#define ATT_TOKEN   "maker:4OEEKYWWlSCbW1VeVrkl4iHXbSCAz0rEGc9muI7"

static int _att_cmd(int argc, char **argv)
{
    // att sock_id asset value
    (void)argc;
    char msg[128] = { 0 };
    unsigned pos = 0;
    if (argc != 4) {
        printf("usage: %s <socket ID> <asset> <value>\n", argv[0]);
        return 1;
    }
    sara_r410m_socket_t sock = { .type = SOCK_UDP };

    strcpy(&msg[pos], ATT_ID);
    pos += strlen(ATT_ID);
    msg[pos++] = '\n';
    strcpy(&msg[pos], ATT_TOKEN);
    pos += strlen(ATT_TOKEN);
    msg[pos++] = '\n';
    sprintf(&msg[pos], "{\"%s\":{\"value\":%s}}", argv[2], argv[3]);
    sock.id = atoi(argv[1]);
    if (sara_r410m_socket_send(&sara_dev, &sock, ATT_IP, ATT_PORT, msg,
                               strlen(msg)) != SARA_R410M_OK) {
        puts("Error: could not send message");
        return 1;
    }
    puts("Message sent!");
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "socket", "Socket operations", _socket_cmd },
    { "sys", "System operations", _sys_cmd },
    { "att", "AllThingsTalk debug commands", _att_cmd },
    { NULL, NULL, NULL }
};


int main(void)
{
    puts("SARA-R410M NB-IoT module driver test");

    if (sara_r410m_init(&sara_dev, &sara_r410m_params[0],
                        &sara_r410m_nbiot_configs[0]) != SARA_R410M_OK) {
        puts("SARA-R410M initialization failed");
        return -1;
    }

    /* start the shell */
    puts("Initialization OK, starting shell now");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
