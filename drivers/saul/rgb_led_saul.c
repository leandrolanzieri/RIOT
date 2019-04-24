/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_saul
 * @{
 *
 * @file
 * @brief       SAUL RGB LED abstraction of GPIOs
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */

#include <string.h>

#include "saul.h"
#include "phydat.h"
#include "periph/gpio.h"
#include "saul/periph.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static int read(const void *dev, phydat_t *res)
{
    const saul_rgb_led_params_t *p = (const saul_rgb_led_params_t *)dev;

    for (unsigned i = 0; i < 3; i++) {
        if (p->gpios[i].pin != GPIO_UNDEF) {
            int inverted = (p->gpios[i].flags & SAUL_GPIO_INVERTED);
            res->val[i] = (gpio_read(p->gpios[i].pin)) ? !inverted : inverted;
        }
    }

    res->unit = UNIT_BOOL;
    res->scale = 0;
    return 3;
}

static int write(const void *dev, phydat_t *state)
{
    const saul_rgb_led_params_t *p = (const saul_rgb_led_params_t *)dev;

    DEBUG("[%s] Values: %d, %d, %d\n", __func__, state->val[0], state->val[1], state->val[2]);

    for (unsigned i = 0; i < 3; i++) {
        if (p->gpios[i].pin != GPIO_UNDEF && state->val[i] != SAUL_RGB_LED_UNDEF_VAL) {
            int inverted = (p->gpios[i].flags & SAUL_GPIO_INVERTED);
            int value = (state->val[i] ? !inverted : inverted);
            gpio_write(p->gpios[i].pin, value);
        }
    }

    return 1;
}

const saul_driver_t rgb_led_saul_driver = {
    .read = read,
    .write = write,
    .type = SAUL_ACT_LED_RGB,
    .undef_value = SAUL_RGB_LED_UNDEF_VAL
};
