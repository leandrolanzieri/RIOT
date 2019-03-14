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
 * @brief       Unittests for the ``link format`` module
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */
#ifndef TESTS_LINK_FORMAT_H
#define TESTS_LINK_FORMAT_H
#include "embUnit/embUnit.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
*  @brief   The entry point of this test suite.
*/
void tests_link_format(void);

/**
 * @brief   Generates tests for link format
 *
 * @return  embUnit tests if successful, NULL if not.
 */
Test *tests_link_format_tests(void);

#ifdef __cplusplus
}
#endif

#endif /* TESTS_LINK_FORMAT_H */
/** @} */
