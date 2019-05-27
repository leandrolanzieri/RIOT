/*
 * Copyright (C) 2019 HAW Hamburg
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
 * @brief       Unittests for the ``runconf_reg`` module
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */
#ifndef TESTS_RUNCONF_REG_H
#define TESTS_RUNCONF_REG_H
#include "embUnit/embUnit.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
*  @brief   The entry point of this test suite.
*/
void tests_runconf_reg(void);

/**
 * @brief   Generates tests for runconf_reg
 *
 * @return  embUnit tests if successful, NULL if not.
 */
Test *tests_runconf_reg_tests(void);

#ifdef __cplusplus
}
#endif

#endif /* TESTS_RUNCONF_REG_H */
/** @} */
