#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "objects/common.h"
#include "liblwm2m.h"
#include "lwm2m_obj_dump.h"
#include "od.h"

#define OUT_BUFFER_LEN  64

int lwm2m_get_set_cmd(int argc, char **argv, lwm2m_client_data_t *client_data)
{

    const bool get = !strcmp(argv[1], "get");

    if (get && argc != 4) {
        printf("usage: %s get <path> <type>\n", argv[0]);
        return 1;
    }

    if (!get && argc != 5) {
        printf("usage: %s set <path> <type> <value>\n", argv[0]);
        printf("where <type> can take the following values:\n");
        printf("  's': string\n  'i': integer\n  'b': boolean\n  'o': opaque \n");
        return 1;
    }

    char *path = argv[2];
    size_t path_len = strlen(path);
    char type = *argv[3];
    int res = -1;

    if (get) {
        uint8_t out_buffer[OUT_BUFFER_LEN] = {0};
        int64_t out_int;
        bool out_bool;

        switch (type) {
        case 's':
            res = lwm2m_get_string_by_path(client_data, path, path_len, (char *)out_buffer,
                                           sizeof(out_buffer));
            if (res != 0) {
                goto get_error_out;
            }

            printf("%s: %s\n", path, out_buffer);
            break;

        case 'o':
            res = lwm2m_get_opaque_by_path(client_data, path, path_len, out_buffer,
                                           sizeof(out_buffer));
            if (res != 0) {
                goto get_error_out;
            }

            printf("%s:\n", path);
            od_hex_dump(out_buffer, sizeof(out_buffer), 0);
            break;

        case 'i':
            res = lwm2m_get_int_by_path(client_data, path, path_len, &out_int);
            if (res != 0) {
                goto get_error_out;
            }

            printf("%s: %" PRIi32 "\n", path, (int32_t)out_int);
            break;

        case 'b':
            res = lwm2m_get_bool_by_path(client_data, path, path_len, &out_bool);
            if (res != 0) {
                goto get_error_out;
            }

            printf("%s: %s\n", path, out_bool ? "True" : "False");
            break;

        default:
            printf("Unknown type\n");
            goto get_error_out;
        }

        return 0;
    }
    else {
        char *value = argv[4];
        size_t value_len = strlen(value);

        switch (type) {
        case 's':
            res = lwm2m_set_string_by_path(client_data, path, path_len, value, value_len);
            if (res != 0) {
                goto set_error_out;
            }
            break;

        case 'o':
            res = lwm2m_set_opaque_by_path(client_data, path, path_len, (uint8_t *)value, value_len);
            if (res != 0) {
                goto set_error_out;
            }
            break;

        case 'i': {
            int64_t val = atoi(value);
            res = lwm2m_set_int_by_path(client_data, path, path_len, val);
            if (res != 0) {
                goto set_error_out;
            }
            break;
        }

        case 'b':
            unsigned val = atoi(value);
            res = lwm2m_set_bool_by_path(client_data, path, path_len, !!val);
            if (res != 0) {
                goto set_error_out;
            }
            break;

        default:
            printf("Unknown type\n");
            goto set_error_out;
        }

        return 0;
    }

get_error_out:
    printf("Could not get %s\n", path);
    return 1;

set_error_out:
    printf("Could not set %s\n", path);
    return 1;

}
