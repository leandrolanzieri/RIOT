/*
 * Copyright (C) Inria 2018
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup   boards_stm32l476g-disco
 * @{
 *
 * @file
 * @brief     Board specific configuration of direct mapped GPIOs
 *
 * @author    Alexandre Abadie <alexandre.abadie@inria.fr>
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
        .name = "/led/unkn/4",
        .pin = LED0_PIN,
        .mode = GPIO_OUT
    },
    {
        .name = "/led/unkn/5",
        .pin = LED1_PIN,
        .mode = GPIO_OUT
    },
    {
        .name = "joystick/0/c",
        .pin = BTN0_PIN,
        .mode = BTN0_MODE
    },
    {
        .name = "joystick/0/l",
        .pin = BTN1_PIN,
        .mode = BTN1_MODE
    },
    {
        .name = "joystick/0/d",
        .pin = BTN2_PIN,
        .mode = BTN2_MODE
    },
    {
        .name = "joystick/0/r",
        .pin = BTN3_PIN,
        .mode = BTN3_MODE
    },
    {
        .name = "joystick/0/u",
        .pin = BTN4_PIN,
        .mode = BTN4_MODE
    },
};

#ifdef __cplusplus
}
#endif

#endif /* GPIO_PARAMS_H */
/** @} */
