/*
* Copyright (C) 2019 HAW Hamburg
*
* This file is subject to the terms and conditions of the GNU Lesser
* General Public License v2.1. See the file LICENSE in the top level
* directory for more details.
*/

#include <string.h>
#include "embUnit.h"
#include "tests-link_format.h"

#include "link_format.h"

#ifdef TESTS_LINK_FORMAT_PRINT
#include <stdio.h>
static void _print_param(link_format_param_t *param)
{
    printf("-- Param: ");
    if (param->type == LINK_FORMAT_PARAM_EXTENSION) {
        printf("%.*s", param->ext_len, param->ext);
        if (param->value) {
            printf(" = %.*s\n", param->value_len, param->value);
        }
        else {
            puts("");
        }
    }
    else {
        printf("%s = %.*s\n", link_format_param_type_to_str(param->type),
               param->value_len, param->value);
    }
}
#endif /* TESTS_LINK_FORMAT_PRINT */

/**
 * @brief Compares two link format parameters
 *
 * @param[in] p1 first parameter to compare
 * @param[in] p2 second parameter to compare
 *
 * @return 0 if parameters are equal
 * @return 1 otherwise
 */
static unsigned _compare_params(link_format_param_t *p1, link_format_param_t *p2)
{
    unsigned result = 1;
    int res;

    if (p1->type != p2->type) {
        goto out;
    }
    if (p1->type == LINK_FORMAT_PARAM_EXTENSION) {
        if (!p1->ext || !p2->ext || (p1->ext_len != p2->ext_len)) {
            goto out;
        }

        res = strncmp(p1->ext, p2->ext, p1->ext_len);
        if (res != 0) {
            goto out;
        }
    }

    if (!p1->value && !p2->value) {
        goto success_out;
    }

    if (!p1->value || !p2->value || (p1->value_len != p2->value_len)) {
        goto out;
    }

    res = strncmp(p1->value, p2->value, p1->value_len);
    if (res != 0) {
        goto out;
    }
success_out:
    result = 0;
out:
    return result;
}

/* This also tests the functions `link_format_add_target` and
 * `link_format_add_param`. */
static void test_link_format_encode_links(void)
{
    const char exp_string[] = "</sensor/temp>;rt=\"temperature\";if=\"sensor\","
                              "</node/info>,</node/ep>;ct=\"40\"";
    link_format_param_t params[] = {
        { .type = LINK_FORMAT_PARAM_RES_TYPE, .value = "temperature" },
        { .type = LINK_FORMAT_PARAM_IF_DESC, .value = "sensor" },
        { .type = LINK_FORMAT_PARAM_CT, .value = "40" }
    };

    link_format_t links[] = {
        { .target = "/sensor/temp", .params = params, .params_len = 2 },
        { .target = "/node/info", .params_len = 0 },
        { .target = "/node/ep", .params = &params[2], .params_len = 1 }
    };

    const size_t exp_size = sizeof(exp_string) - 1;
    char output[exp_size + 1];
    size_t pos = 0;
    ssize_t res = 0;

    res = link_format_encode_link(&links[0], output, sizeof(output));
    pos += res;

    for (unsigned i = 1; i < sizeof(links) / sizeof(links[0]); i++) {
        res = link_format_add_link_separator(&output[pos], sizeof(output) - pos);
        if (res <= 0) {
            break;
        }
        pos += res;

        res = link_format_encode_link(&links[i], &output[pos], sizeof(output) - pos);
        if (res <= 0) {
            break;
        }
        pos += res;
    }
    output[pos++] = '\0';

#ifdef TESTS_LINK_FORMAT_PRINT
    puts("\n========================================");
    puts("[Test: encode_links]");
    puts("---------------------");
    printf("Encoded links: %s\n", output);
#endif

    TEST_ASSERT_EQUAL_STRING(exp_string, output);
}

/* This also tests the functions `link_format_get_link`,
 * `link_format_get_target` and `link_format_get_param` */
static void test_link_format_decode_links(void)
{
    char input_string[] = "</sensors>;ct=40;title=\"Sensor Index\","
                          "</sensors/temp>;rt=\"temperature-c\";if=\"sensor\","
                          "</sensors/light>;rt=\"light-lux\";if=\"sensor\","
                          "<http://www.example.com/sensors/t123>;"
                          "anchor=\"/sensors/temp\";rel=\"describedby\";sz=1234,"
                          "</t>;anchor=\"/sensors/temp\";rel=\"alternate\";s,"
                          "</riot/board>,</riot/info>";

    link_format_param_t exp_params[] = {
        { .type = LINK_FORMAT_PARAM_CT, .value = "40",
          .value_len = sizeof("40") - 1 },
        { .type = LINK_FORMAT_PARAM_TITLE, .value = "Sensor Index",
          .value_len = sizeof("Sensor Index") - 1 },
        { .type = LINK_FORMAT_PARAM_RES_TYPE, .value = "temperature-c",
          .value_len = sizeof("temperature-c") - 1 },
        { .type = LINK_FORMAT_PARAM_IF_DESC, .value = "sensor",
          .value_len = sizeof("sensor") - 1 },
        { .type = LINK_FORMAT_PARAM_RES_TYPE, .value = "light-lux",
          .value_len = sizeof("light-lux") - 1 },
        { .type = LINK_FORMAT_PARAM_IF_DESC, .value = "sensor",
          .value_len = sizeof("sensor") - 1 },
        { .type = LINK_FORMAT_PARAM_ANCHOR, .value = "/sensors/temp",
          .value_len = sizeof("/sensors/temp") - 1},
        { .type = LINK_FORMAT_PARAM_REL_TYPE, .value = "describedby",
          .value_len = sizeof("describedby") - 1 },
        { .type = LINK_FORMAT_PARAM_SIZE, .value = "1234",
          .value_len = sizeof("1234") - 1 },
        { .type = LINK_FORMAT_PARAM_ANCHOR,   .value = "/sensors/temp",
          .value_len = sizeof("/sensors/temp") - 1 },
        { .type = LINK_FORMAT_PARAM_REL_TYPE, .value = "alternate",
          .value_len = sizeof("alternate") - 1 },
        { .type = LINK_FORMAT_PARAM_EXTENSION, .ext = "s", .ext_len = 1,
          .value_len = 0 }
    };
    const char *exp_targets[] = {
        "/sensors", "/sensors/temp", "/sensors/light",
        "http://www.example.com/sensors/t123", "/t", "/riot/board", "/riot/info"
    };

    const unsigned exp_links_numof = sizeof(exp_targets) / sizeof(exp_targets[0]);
    const unsigned exp_params_numof = sizeof(exp_params) / sizeof(exp_params[0]);

    link_format_t out_links[exp_links_numof];
    link_format_param_t out_params[exp_params_numof];

    unsigned links_numof = link_format_decode_links(out_links, exp_links_numof,
                                                    out_params, exp_params_numof,
                                                    input_string,
                                                    strlen(input_string));
#ifdef TESTS_LINK_FORMAT_PRINT
    puts("\n========================================");
    puts("[Test: decode_links]");
    printf("- Amount of decoded links: %u\n", links_numof);
#endif
    unsigned params_numof = 0;
    TEST_ASSERT_EQUAL_INT(exp_links_numof, links_numof);

    for (unsigned i = 0; i < exp_links_numof; i++) {
        TEST_ASSERT(!strncmp(exp_targets[i], out_links[i].target, out_links[i].target_len));

#ifdef TESTS_LINK_FORMAT_PRINT
        puts("---------------------");
        puts("New link:");
        printf("- Target: %.*s\n", out_links[i].target_len, out_links[i].target);
        printf("- Number of parameters: %d\n", out_links[i].params_len);
#endif
        for (unsigned j = 0; j < out_links[i].params_len; j++) {
            TEST_ASSERT(!_compare_params(&out_links[i].params[j],
                                         &exp_params[params_numof]));
            params_numof++;
#ifdef TESTS_LINK_FORMAT_PRINT
            _print_param(&out_links[i].params[j]);
#endif
        }
    }
}

Test *tests_link_format_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_link_format_encode_links),
        new_TestFixture(test_link_format_decode_links)
    };

    EMB_UNIT_TESTCALLER(link_format_tests, NULL, NULL, fixtures);

    return (Test *)&link_format_tests;
}

void tests_link_format(void)
{
    TESTS_RUN(tests_link_format_tests());
}
