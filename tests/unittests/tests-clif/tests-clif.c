/*
* Copyright (C) 2019 HAW Hamburg
*
* This file is subject to the terms and conditions of the GNU Lesser
* General Public License v2.1. See the file LICENSE in the top level
* directory for more details.
*/

#include <string.h>
#include "embUnit.h"
#include "tests-clif.h"

#include "clif.h"

#ifdef TESTS_CLIF_PRINT
#include <stdio.h>
static void _print_param(clif_param_t *param)
{
    if (param->key) {
        printf("-- Param: ");
        printf("%.*s", param->key_len, param->key);

        if (param->value) {
            printf(" = %.*s\n", param->value_len, param->value);
        }
        else {
            puts("");
        }
    }
}
#endif /* TESTS_CLIF_PRINT */

/**
 * @brief Compares two link format parameters
 *
 * @param[in] p1 first parameter to compare
 * @param[in] p2 second parameter to compare
 *
 * @return 0 if parameters are equal
 * @return 1 otherwise
 */
static unsigned _compare_params(clif_param_t *p1, clif_param_t *p2)
{
    unsigned result = 1;
    int res;

    if (p1->key_len != p2->key_len) {
        goto out;
    }

    if (strncmp(p1->key, p2->key, p1->key_len)) {
        goto out;
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

/* This also tests the functions `clif_add_target` and
 * `clif_add_param`. */
static void test_clif_encode_links(void)
{
    const char exp_string[] = "</sensor/temp>;rt=\"temperature\";if=\"sensor\","
                              "</node/info>,</node/ep>;ct=\"40\"";
    clif_param_t params[] = {
        { .key = "rt", .value = "temperature" },
        { .key = "if", .value = "sensor" },
        { .key = "ct", .value = "40" }
    };

    clif_t links[] = {
        { .target = "/sensor/temp", .params = params, .params_len = 2 },
        { .target = "/node/info", .params_len = 0 },
        { .target = "/node/ep", .params = &params[2], .params_len = 1 }
    };

    const size_t exp_size = sizeof(exp_string) - 1;
    char output[exp_size + 1];
    size_t pos = 0;
    ssize_t res = 0;

    /* first test with NULL output to check the needed bytes */
    res = clif_encode_link(&links[0], NULL, 0);
    pos += res;

    for (unsigned i = 1; i < sizeof(links) / sizeof(links[0]); i++) {
        res = clif_add_link_separator(NULL, 0);
        if (res <= 0) {
            break;
        }
        pos += res;

        res = clif_encode_link(&links[i],NULL, 0);
        if (res <= 0) {
            break;
        }
        pos += res;
    }

    TEST_ASSERT_EQUAL_INT(exp_size, pos);

    /* now actually encode the links */
    pos = 0;
    res = clif_encode_link(&links[0], output, sizeof(output));
    pos += res;

    for (unsigned i = 1; i < sizeof(links) / sizeof(links[0]); i++) {
        res = clif_add_link_separator(&output[pos], sizeof(output) - pos);
        if (res <= 0) {
            break;
        }
        pos += res;

        res = clif_encode_link(&links[i], &output[pos], sizeof(output) - pos);
        if (res <= 0) {
            break;
        }
        pos += res;
    }
    output[pos++] = '\0';

#ifdef TESTS_CLIF_PRINT
    puts("\n========================================");
    puts("[Test: encode_links]");
    puts("---------------------");
    printf("Encoded links: %s\n", output);
#endif

    TEST_ASSERT_EQUAL_STRING(exp_string, output);
    TEST_ASSERT_EQUAL_INT(exp_size, pos - 1); /* do not count '\0' */
}

/* This also tests the functions `clif_get_link`,
 * `clif_get_target` and `clif_get_param` */
static void test_clif_decode_links(void)
{
    /* string to decode */
    char input_string[] = "</sensors>;ct=40;title=\"Sensor Index\","
                          "</sensors/temp>;rt=\"temperature-c\";if=\"sensor\","
                          "</sensors/light>;rt=\"light-lux\";if=\"sensor\","
                          "<http://www.example.com/sensors/t123>;"
                          "anchor=\"/sensors/temp\";rel=\"describedby\";sz=1234,"
                          "</t>;anchor=\"/sensors/temp\";rel=\"alternate\";s,"
                          "</riot/board>,</riot/info>;obs";

    /* ordered expected types to be decoded */
    clif_param_type_t exp_types[] = {
        CLIF_PARAM_CT, CLIF_PARAM_TITLE, CLIF_PARAM_RT, CLIF_PARAM_IF,
        CLIF_PARAM_RT, CLIF_PARAM_IF, CLIF_PARAM_ANCHOR, CLIF_PARAM_REL,
        CLIF_PARAM_SZ, CLIF_PARAM_ANCHOR, CLIF_PARAM_REL, CLIF_PARAM_EXT,
        CLIF_PARAM_OBS
    };

    /* ordered amount of expected parameters per link to be decoded */
    unsigned exp_param_numof[] = { 2, 2, 2, 3, 3, 0, 1 };

    /* ordered expected parameters to be decoded */
    clif_param_t exp_params[] = {
        { .key = "ct", .key_len = 2, .value = "40", .value_len = sizeof("40") - 1 },
        { .key = "title", .key_len = 5, .value = "Sensor Index", .value_len = sizeof("Sensor Index") - 1 },
        { .key = "rt", .key_len = 2, .value = "temperature-c", .value_len = sizeof("temperature-c") - 1 },
        { .key = "if", .key_len = 2, .value = "sensor", .value_len = sizeof("sensor") - 1 },
        { .key = "rt", .key_len = 2, .value = "light-lux", .value_len = sizeof("light-lux") - 1 },
        { .key = "if", .key_len = 2, .value = "sensor", .value_len = sizeof("sensor") - 1 },
        { .key = "anchor", .key_len = 6, .value = "/sensors/temp", .value_len = sizeof("/sensors/temp") - 1},
        { .key = "rel", .key_len = 3, .value = "describedby", .value_len = sizeof("describedby") - 1 },
        { .key = "sz", .key_len = 2, .value = "1234", .value_len = sizeof("1234") - 1 },
        { .key = "anchor", .key_len = 6,   .value = "/sensors/temp", .value_len = sizeof("/sensors/temp") - 1 },
        { .key = "rel", .key_len = 3, .value = "alternate", .value_len = sizeof("alternate") - 1 },
        { .key = "s", .key_len = 1, .value_len = 0 },
        { .key = "obs", .key_len = 3, .value_len = 0 }
    };

    /* ordered expected targets to be decoded */
    const char *exp_targets[] = {
        "/sensors", "/sensors/temp", "/sensors/light",
        "http://www.example.com/sensors/t123", "/t", "/riot/board", "/riot/info"
    };

    const unsigned exp_links_numof = sizeof(exp_targets) / sizeof(exp_targets[0]);
    const unsigned exp_params_numof = sizeof(exp_params) / sizeof(exp_params[0]);
    const size_t input_len = sizeof(input_string) - 1;

    clif_t out_link;
    char *pos = input_string;
    unsigned links_numof = 0;

    /* first test without parameters array, to test the expected parameters
     * functionality */
    do {
        ssize_t res = clif_decode_link(&out_link, NULL, 0, pos,
                                       input_len - (pos - input_string));
        if (res < 0) {
            break;
        }
        pos += res;

        /* check expected target */
        TEST_ASSERT(!strncmp(exp_targets[links_numof], out_link.target, out_link.target_len));

        /* check expected amount of parameters */
        TEST_ASSERT_EQUAL_INT(exp_param_numof[links_numof], out_link.params_len);
        links_numof++;
    } while (pos < input_string + sizeof(input_string));

#ifdef TESTS_CLIF_PRINT
    puts("\n========================================");
    puts("[Test: decode_links]");
    printf("- Amount of decoded links: %u\n", links_numof);
#endif
    TEST_ASSERT(exp_links_numof == links_numof);

    /* now decode again but saving the parameters */
    clif_param_t out_params[exp_params_numof];
    pos = input_string;
    unsigned params_numof = 0;
    do {
        ssize_t res = clif_decode_link(&out_link, &out_params[params_numof],
                                       exp_params_numof - params_numof, pos,
                                       input_len - (pos - input_string));

        if (res < 0) {
            break;
        }
        pos += res;
#ifdef TESTS_CLIF_PRINT
        puts("---------------------");
        puts("New link:");
        printf("- Target: %.*s\n", out_link.target_len, out_link.target);
        printf("- Number of parameters: %d\n", out_link.params_len);
#endif

        for (unsigned i = 0; i < out_link.params_len; i++) {
            TEST_ASSERT(!_compare_params(&out_link.params[i],
                                         &exp_params[params_numof]));
            clif_param_type_t type = clif_get_param_type(out_link.params[i].key,
                                                         out_link.params[i].key_len);
            TEST_ASSERT_EQUAL_INT(exp_types[params_numof], type);
            params_numof++;
#ifdef TESTS_CLIF_PRINT
            _print_param(&out_link.params[i]);
#endif
        }
    } while (pos < input_string + sizeof(input_string));
    TEST_ASSERT_EQUAL_INT(exp_params_numof, params_numof);
}

Test *tests_clif_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_clif_encode_links),
        new_TestFixture(test_clif_decode_links)
    };

    EMB_UNIT_TESTCALLER(clif_tests, NULL, NULL, fixtures);

    return (Test *)&clif_tests;
}

void tests_clif(void)
{
    TESTS_RUN(tests_clif_tests());
}
