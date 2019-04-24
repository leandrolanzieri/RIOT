/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_sodaq-sara-aff
 * @{
 *
 * @file
 * @brief       Board specific configuration of RGB LEDs
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */

#ifndef RGB_LED_PARAMS_H
#define RGB_LED_PARAMS_H

#include "board.h"
#include "saul/periph.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief    RGB LED configuration
 */
static const saul_rgb_led_params_t saul_rgb_led_params[] =
{
    {
        .gpios = {
            [SAUL_RGB_LED_RED] = {
                .pin = LED1_PIN,
                .mode = GPIO_OUT,
                .flags = (SAUL_GPIO_INVERTED | SAUL_GPIO_INIT_CLEAR)
            },
            [SAUL_RGB_LED_GREEN] = {
                .pin = LED2_PIN,
                .mode = GPIO_OUT,
                .flags = (SAUL_GPIO_INVERTED | SAUL_GPIO_INIT_CLEAR)
            },
            [SAUL_RGB_LED_BLUE] = {
                .pin = LED3_PIN,
                .mode = GPIO_OUT,
                .flags = (SAUL_GPIO_INVERTED | SAUL_GPIO_INIT_CLEAR)
            }
        }
    }
};

/**
 * @brief Extra information for SAUL registry
 */
static const saul_reg_info_t saul_rgb_led_info[] =
{
    { .name = "rgb/0" }
};

#ifdef __cplusplus
}
#endif

#endif /* RGB_LED_PARAMS_H */
/** @} */
