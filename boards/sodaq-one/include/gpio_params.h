/*
 * Copyright (C) 2017 Kees Bakker, SODAQ
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_sodaq-one
 * @{
 *
 * @file
 * @brief       Board specific configuration of direct mapped GPIOs
 *
 * @author      Kees Bakker <kees@sodaq.com>
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
static const  saul_gpio_params_t saul_gpio_params[] =
{
    {
        .pin = LED0_PIN,
        .mode = GPIO_OUT
    },
    {
        .pin = LED1_PIN,
        .mode = GPIO_OUT
    },
    {
        .pin = LED2_PIN,
        .mode = GPIO_OUT
    },
    {
        .pin  = BTN0_PIN,
        .mode = BTN0_MODE,
        .flags = SAUL_GPIO_INVERTED
    },
};

/**
 * @brief GPIO information for SAUL registry
 */
static const saul_reg_info_t saul_gpio_info[] =
{
    { .name = "LED Red" },
    { .name = "LED Green" },
    { .name = "LED Blue" },
    { .name = "Button" }
};

#ifdef __cplusplus
}
#endif

#endif /* GPIO_PARAMS_H */
/** @} */
