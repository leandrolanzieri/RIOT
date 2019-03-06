/*
 * Copyright (C) 2015 Martin Landsmann
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @addtogroup  unittests
 * @{
 *
 * @file
 * @brief       Unittests for the ``pm_hook`` module
 *
 * @author      Martin Landsmann <Martin.Landsmann@HAW-Hamburg.de>
 */
#ifndef TESTS_PM_HOOK
#define TESTS_PM_HOOK
#include "embUnit/embUnit.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
*  @brief   The entry point of this test suite.
*/
void tests_pm_hook(void);

/**
 * @brief   Generates tests for pm_hook
 *
 * @return  embUnit tests if successful, NULL if not.
 */
Test *tests_pm_hook_tests(void);

#ifdef __cplusplus
}
#endif

#endif /* TESTS_PM_HOOK */
/** @} */
