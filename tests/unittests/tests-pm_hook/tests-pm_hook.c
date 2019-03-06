/*
* Copyright (C) 2015 Martin Landsmann
*
* This file is subject to the terms and conditions of the GNU Lesser
* General Public License v2.1. See the file LICENSE in the top level
* directory for more details.
*/


#include <stdio.h>
#include <string.h>
#include "embUnit.h"
#include "tests-pm_hook.h"

#include "pm_hook.h"

static int _save_success(void *ctx)
{
    *(int *)ctx += 1;
    return 0;
}

static int _restore_success(void *ctx)
{
    *(int *)ctx -= 1;
    return 0;
}

static int _fail(void *ctx)
{
    (void)ctx;
    return 1;
}

static void test_pm_hook_register_save_success_restore_fail(void)
{
    int ctx = 0;
    pm_hook_t hook1 = { .save = _save_success, .restore = _restore_success,
                        .ctx = &ctx };

    pm_hook_t hook2 = { .save = _save_success, .restore = _fail,
                        .ctx = &ctx };

    pm_hook_init();
    pm_hook_register(&hook1);
    pm_hook_register(&hook2);

    TEST_ASSERT(!pm_hook_save());
    TEST_ASSERT_EQUAL_INT(2, ctx);

    pm_hook_restore();
    TEST_ASSERT_EQUAL_INT(1, ctx);
}


static void test_pm_hook_register_save_fail_restore_success(void)
{
    int ctx = 0;
    pm_hook_t hook1 = { .save = _save_success, .restore = _restore_success,
                        .ctx = &ctx };

    pm_hook_t hook2 = { .save = _fail, .restore = _restore_success,
                        .ctx = &ctx };

    pm_hook_init();
    pm_hook_register(&hook1);
    pm_hook_register(&hook2);

    TEST_ASSERT(pm_hook_save());
    TEST_ASSERT_EQUAL_INT(1, ctx);

    pm_hook_restore();
    TEST_ASSERT_EQUAL_INT(-1, ctx);
}


Test *tests_pm_hook_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_pm_hook_register_save_success_restore_fail),
        new_TestFixture(test_pm_hook_register_save_fail_restore_success),
    };

    EMB_UNIT_TESTCALLER(pm_hook_tests, NULL, NULL, fixtures);

    return (Test *)&pm_hook_tests;
}

void tests_pm_hook(void)
{
    TESTS_RUN(tests_pm_hook_tests());
}
