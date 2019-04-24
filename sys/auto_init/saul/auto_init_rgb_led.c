/*
 * Copyright (C) 2019
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/*
 * @ingroup     sys_auto_init_saul
 * @{
 *
 * @file
 * @brief       Auto initialization of SAUL RGB LEDs
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */

#ifdef MODULE_SAUL_RGB_LED

#include "log.h"
#include "saul_reg.h"
#include "saul/periph.h"
#include "rgb_led_params.h"
#include "periph/gpio.h"

/**
 * @brief   Define the number of configured RGB LEDs
 */
#define SAUL_RGB_LED_NUMOF (sizeof(saul_rgb_led_params)/sizeof(saul_rgb_led_params[0]))

/**
 * @brief   Define the number of SAUL info for the RGB LEDs
 */
#define SAUL_RGB_LED_INFO_NUMOF (sizeof(saul_rgb_led_info) / sizeof(saul_rgb_led_info[0]))

/**
 * @brief   Memory for the registry entries
 */
static saul_reg_t saul_reg_entries[SAUL_RGB_LED_NUMOF];

/**
 * @brief   Reference the driver driver struct
 */
extern saul_driver_t rgb_led_saul_driver;


void auto_init_rgb_led(void)
{
    assert(SAUL_RGB_LED_NUMOF == SAUL_RGB_LED_INFO_NUMOF);
    phydat_t s;

    for (unsigned i = 0; i < SAUL_RGB_LED_NUMOF; i++) {
        const saul_rgb_led_params_t *p = &saul_rgb_led_params[i];

        LOG_DEBUG("[auto_init_saul] initializing RGB LED #%u\n", i);

        /* configure registry entry */
        saul_reg_entries[i].dev = (void *)p;
        saul_reg_entries[i].name = saul_rgb_led_info[i].name;
        saul_reg_entries[i].driver = &rgb_led_saul_driver;

        /* initialize each channel */
        for (unsigned ch = 0; ch < 3; ch ++) {
            if (p->gpios[ch].pin != GPIO_UNDEF) {
                gpio_init(p->gpios[ch].pin, GPIO_OUT);
                /* set initial pin state if configured */
                if (p->gpios[ch].flags & (SAUL_GPIO_INIT_CLEAR | SAUL_GPIO_INIT_SET)) {
                    s.val[ch] = (p->gpios[ch].flags & SAUL_GPIO_INIT_SET);
                }
                else {
                    s.val[ch] = SAUL_RGB_LED_UNDEF_VAL;
                }
            }
            else {
                s.val[ch] = SAUL_RGB_LED_UNDEF_VAL;
            }
        }
        saul_reg_entries[i].driver->write(p, &s);

        /* add to registry */
        saul_reg_add(&(saul_reg_entries[i]));
    }
}
#else
typedef int dont_be_pedantic;
#endif /* MODULE_SAUL_RGB_LED */
