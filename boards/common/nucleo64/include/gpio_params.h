/*
 * Copyright (C) Inria 2016
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_common_nucleo
 * @{
 *
 * @file
 * @brief       Board specific configuration of direct mapped GPIOs
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 * @author      Sebastian Meiling <s@mlng.net>
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
#ifdef AUTO_INIT_LED0
    {
        .name = "led/green/0",
        .pin = LED0_PIN,
        .mode = GPIO_OUT
    },
#endif
    {
        .name = "button/1",
        .pin  = BTN0_PIN,
        .mode = BTN0_MODE,
#ifndef CPU_MODEL_STM32L433RC
        .flags = SAUL_GPIO_INVERTED
#endif
    },
};

#ifdef __cplusplus
}
#endif

#endif /* GPIO_PARAMS_H */
/** @} */
