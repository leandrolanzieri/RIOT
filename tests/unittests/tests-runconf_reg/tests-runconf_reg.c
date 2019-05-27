/*
* Copyright (C) 2019 HAW Hamburg
*
* This file is subject to the terms and conditions of the GNU Lesser
* General Public License v2.1. See the file LICENSE in the top level
* directory for more details.
*/

#include <stdio.h>
#include <string.h>
#include "embUnit.h"
#include "tests-runconf_reg.h"

#include "runconf/runconf_reg.h"

static void _test_setup(void) {
    runconf_reg_init();
}

static void test_runconf_reg_add_get_groups(void)
{
    _test_setup();

    runconf_reg_group_t my_module_group = {
        .name = "my_module"
    };

    runconf_reg_group_t another_group = {
        .name = "another"
    };

    runconf_reg_add_group(&my_module_group);
    runconf_reg_add_group(&another_group);

    runconf_reg_group_t *res = runconf_reg_get_group("my_module", -1);
    TEST_ASSERT(res == &my_module_group);
}

static void test_runconf_parse_key(void)
{
    _test_setup();
    char input_path[80] = { 0 };
    char *input_argv[] = { "module", "group", "subgroup", "key" };
    unsigned exp_argc = sizeof(input_argv) / sizeof(input_argv[0]);
    int out_argc = 0;
    char *out_argv[exp_argc];

    strcat(input_path, input_argv[0]);
    for (unsigned i = 1; i < exp_argc; i++) {
        strcat(input_path, RUNCONF_REG_KEY_SEPARATOR);
        strcat(input_path, input_argv[i]);
    }

    TEST_ASSERT_EQUAL_INT(runconf_reg_parse_key(input_path, &out_argc, out_argv,
                                               exp_argc),
                          0);
    TEST_ASSERT_EQUAL_INT(out_argc, exp_argc);
    for (int i = 0; i < out_argc; i++) {
        TEST_ASSERT_EQUAL_STRING(input_argv[i], out_argv[i]);
    }
}

Test *tests_runconf_reg_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_runconf_reg_add_get_groups),
        new_TestFixture(test_runconf_parse_key)
    };

    EMB_UNIT_TESTCALLER(runconf_reg_tests, NULL, NULL, fixtures);

    return (Test *)&runconf_reg_tests;
}

void tests_runconf_reg(void)
{
    TESTS_RUN(tests_runconf_reg_tests());
}
