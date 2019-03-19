/*
 * Copyright (C) 2008, 2009, 2010 Kaspar Schleiser <kaspar@schleiser.de>
 * Copyright (C) 2013 INRIA
 * Copyright (C) 2013 Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
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
 * @brief       Default application that shows a lot of functionality of RIOT
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "shell.h"
#include "shell_commands.h"
#include "coral.h"

#define ENCODING_BUF_SIZE (32)

static unsigned char ebuf[ENCODING_BUF_SIZE];

static int _dump_cmd(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    puts("===== DUMP =====");

    puts("> Encoded buffer");
    for (unsigned i = 0; i < sizeof(ebuf); i++) {
        printf("%2x ", ebuf[i]);
    }
    puts("");
    return 0;
}

static shell_command_t shell_commands[] = {
    { "dump", "Dumps the CBOR buffer", _dump_cmd },
    { NULL, NULL, NULL }
};

int main(void)
{

    puts("Some cn-cbor tests in RIOT");

    // memset(ebuf, '\0', sizeof(ebuf));
    // memarray_init(&storage, block_storage_data, sizeof(cn_cbor), CBOR_NUM_BLOCKS);

    // cn_cbor *root = cn_cbor_array_create(&ct, NULL);
    // cn_cbor *val;

    // val = cn_cbor_int_create(2, &ct, NULL);
    // cn_cbor_array_append(root, val, NULL);

    // val = cn_cbor_string_create("text", &ct, NULL);
    // cn_cbor_array_append(root, val, NULL);

    // val = cn_cbor_int_create(1, &ct, NULL);
    // cn_cbor_array_append(root, val, NULL);

    // pos += cn_cbor_encoder_write(ebuf, pos, sizeof(ebuf), root);
    ///////////////////////////////////////

    coral_element_t coral_doc;
    coral_create_document(&coral_doc);

    coral_element_t sensor_resource;
    coral_create_link(&sensor_resource, "hosts", "coap://[fd00:dead:beef::1]/sens/temperature");
    coral_append_element(&coral_doc, &sensor_resource);

    coral_element_t sensor_desc;
    coral_create_link(&sensor_desc, "describedby", "http://example.com/phys.owl#Temperature");
    coral_append_element(&sensor_resource, &sensor_desc);

    coral_print_structure(&coral_doc);
    coral_encode(&coral_doc, ebuf, ENCODING_BUF_SIZE);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
