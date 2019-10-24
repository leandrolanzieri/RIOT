/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test for Kconfig configuration
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include "app.h"

int main(void)
{
    puts(CONFIG_APP_MSG_1_TEXT);

#ifdef CONFIG_APP_MSG_2
    puts(CONFIG_APP_MSG_2_TEXT);
#endif /* CONFIG_APP_MSG_2 */
    return 0;
}
