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
    runconf_reg_group_t my_module_group = {
        .name = "my_module"
    };

    runconf_reg_group_t another_group = {
        .name = "another"
    };

    runconf_reg_add_group(&my_module_group);
    runconf_reg_add_group(&another_group);

    runconf_reg_group_t *res = runconf_reg_get_group(my_module_group.name,
                                                     strlen(my_module_group.name));
    TEST_ASSERT(res == &my_module_group);
}

static void test_runconf_get_key(void)
{
    const runconf_reg_key_t keys[] = {
        {
            .name = "key_1",
            .type = RUNCONF_REG_TYPE_BOOL
        },
        {
            .name = "key_2",
            .type = RUNCONF_REG_TYPE_BOOL
        }
    };

    runconf_reg_group_t group = {
        .name = "get_key_module",
        .keys = keys,
        .keys_numof = sizeof(keys) / sizeof(*keys)
    };

    char name[24] = { '\0' };

    runconf_reg_add_group(&group);

    for (unsigned i = 0; i < (sizeof(keys) / sizeof(*keys)); i++) {
        strcat(name, group.name);
        strcat(name, RUNCONF_REG_KEY_SEPARATOR);
        strcat(name, keys[i].name);

        const runconf_reg_key_t *key;
        runconf_reg_group_t *found_group;
        found_group = runconf_reg_get_key(name, strlen(name), &key);

        TEST_ASSERT(&group == found_group);
        TEST_ASSERT(&keys[i] == key);

        name[0] = '\0';
    }
}

Test *tests_runconf_reg_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_runconf_reg_add_get_groups),
        new_TestFixture(test_runconf_get_key)
    };

    EMB_UNIT_TESTCALLER(runconf_reg_tests, _test_setup, NULL, fixtures);

    return (Test *)&runconf_reg_tests;
}

void tests_runconf_reg(void)
{
    TESTS_RUN(tests_runconf_reg_tests());
}
