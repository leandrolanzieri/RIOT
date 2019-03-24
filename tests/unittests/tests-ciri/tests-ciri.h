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
 * @brief       Unittests for the ``CIRI`` module
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */
#ifndef TESTS_CIRI
#define TESTS_CIRI
#include "embUnit/embUnit.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
*  @brief   The entry point of this test suite.
*/
void tests_ciri(void);

/**
 * @brief   Generates tests for CIRI
 *
 * @return  embUnit tests if successful, NULL if not.
 */
Test *tests_ciri_tests(void);

#ifdef __cplusplus
}
#endif

#endif /* TESTS_CIRI */
/** @} */
