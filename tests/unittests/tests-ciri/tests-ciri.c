/*
* Copyright (C) 2019 HAW Hamburg
*
* This file is subject to the terms and conditions of the GNU Lesser
* General Public License v2.1. See the file LICENSE in the top level
* directory for more details.
*/

#define TEST_CIRI_SHOW_OUTPUT (0) /**< set if detailed info is displayed */

#if (TEST_CIRI_SHOW_OUTPUT == 1)
#include <stdio.h>
#endif
#include <string.h>
#include "embUnit.h"
#include "tests-ciri.h"

#include "ciri.h"

static void test_ciri_check_format(void)
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
    
    // TODO: link these using module
    scheme.next = &host_ip;
    host_ip.next = &port;
    port.next = &path1;
    path1.next = &path2;

    TEST_ASSERT_EQUAL_INT(ciri_is_well_formed(&scheme), CIRI_RET_OK);
}


Test *tests_ciri_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_ciri_check_format),
    };

    EMB_UNIT_TESTCALLER(ciri_tests, NULL, NULL, fixtures);

    return (Test *)&ciri_tests;
}

void tests_ciri(void)
{
    TESTS_RUN(tests_ciri_tests());
}
