/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */



#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "shell.h"
#include "shell_commands.h"
#include "net/nanocoap.h"
#include "coral.h"

#define ENCODING_BUF_SIZE (512)

static unsigned char ebuf[ENCODING_BUF_SIZE];

static int _dump_cmd(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    puts("===== DUMP =====");

    puts("> Encoded buffer");
    for (unsigned i = 0; i < sizeof(ebuf); i++) {
        if (!(i % 27)) {
            puts("");
        }
        printf("%.2x ", ebuf[i]);
    }
    puts("");
    return 0;
}

static shell_command_t shell_commands[] = {
    { "dump", "Dumps the encoded CoRAL document", _dump_cmd },
    { NULL, NULL, NULL }
};

int main(void)
{

    puts("Some cn-cbor tests in RIOT");
    printf("Size of a coral element: %d\n", sizeof(coral_element_t));

    /* create CoRAL root element */
    coral_element_t coral_doc;
    coral_link_target_t link_target;
    coral_create_document(&coral_doc);

    /* create a CoRAL link of temperature sensor and append to doc */
    coral_element_t sensor_resource;
    coral_literal_string(&link_target, "/sense/temp");
    coral_create_link(&sensor_resource, "temperature", &link_target);
    coral_append_element(&coral_doc, &sensor_resource);

    /* create a CoRAL link that describes the previous link */
    coral_element_t sensor_desc;
    coral_literal_string(&link_target, "http://example.com/phys.owl#Temperature");
    coral_create_link(&sensor_desc, "describedby", &link_target);
    coral_append_element(&sensor_resource, &sensor_desc);

    /* create a CoRAL embedded representation of the previous link */
    coral_element_t embedded_rep;
    uint8_t _rep[] = { 0x12, 0x34, 0x56, 0x78 };
    coral_create_rep(&embedded_rep, _rep, sizeof(_rep));
    coral_append_element(&sensor_resource, &embedded_rep);
    
    /* create a CoRAL form of a heater actuator */
    coral_element_t actuator_resource;
    coral_form_target_t form_target;
    coral_literal_string(&form_target, "/act/heat");
    coral_create_form(&actuator_resource, "heater", COAP_POST, &form_target);
    coral_append_element(&coral_doc, &actuator_resource);

    /* create a form field for previous form */
    coral_element_t actuator_field;
    coral_literal_t field_val;
    coral_literal_string(&field_val, "field_val");
    coral_create_form_field(&actuator_field, "field_name", &field_val);
    coral_append_element(&actuator_resource, &actuator_field);

    /* print the structure in memory */
    coral_print_structure(&coral_doc);

    /* encode the document into CBOR buffer */
    size_t used = coral_encode(&coral_doc, ebuf, ENCODING_BUF_SIZE);

    /* parse the buffer again into CoRAL and print it */
    (void)used;
    puts("\nNow parsing the encoded output\n\n");
    coral_element_t out[6];
    coral_decode(out, sizeof(out) / sizeof(out[0]), ebuf, used);
    coral_print_structure(out);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
