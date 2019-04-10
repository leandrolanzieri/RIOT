/*
 * Copyright (C) 2019 Inria
 *               2019 Freie Universität Berlin
 *               2019 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup   boards_pyboard
 * @{
 *
 * @file
 * @brief     Board specific configuration of direct mapped GPIOs
 *
 * @author    Kaspar Schleiser <kaspar@schleiser.de>
 */

#ifndef GPIO_PARAMS_H
#define GPIO_PARAMS_H

#include "board.h"
#include "saul/periph.h"
#include "saul_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief    GPIO pin configuration
 */
static const saul_gpio_params_t saul_gpio_params[] =
{
    {
        .pin = LED0_PIN,
        .mode = GPIO_OUT
    },
    {
        .pin = BTN_B1_PIN,
        .mode = GPIO_IN_PU,
        .flags = SAUL_GPIO_INVERTED
    }
};

/**
 * @brief GPIO information for SAUL registry
 */
static const saul_reg_info_t saul_gpio_info[] =
{
    { .name = "LD1" },
    { .name = "Button(B1 User)" }
};

#ifdef __cplusplus
}
#endif

#endif /* GPIO_PARAMS_H */
/** @} */
