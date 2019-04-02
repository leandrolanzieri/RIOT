/*
 * Copyright (C) 2017 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_mbed_lpc1768
 * @{
 *
 * @file
 * @brief       Board specific configuration of direct mapped GPIOs
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef GPIO_PARAMS_H
#define GPIO_PARAMS_H

#include "board.h"
#include "saul/periph.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief    GPIO pin configuration
 */
static const  saul_gpio_params_t saul_gpio_params[] =
{
    {
        .name = "led/unkn/0",
        .pin = LED0_PIN,
        .mode = GPIO_OUT
    },
    {
        .name = "led/unkn/1",
        .pin = LED1_PIN,
        .mode = GPIO_OUT
    },
    {
        .name = "led/unkn/2",
        .pin = LED2_PIN,
        .mode = GPIO_OUT
    },
    {
        .name = "led/unkn/3",
        .pin = LED3_PIN,
        .mode = GPIO_OUT
    }
};

#ifdef __cplusplus
}
#endif

#endif /* GPIO_PARAMS_H */
/** @} */
