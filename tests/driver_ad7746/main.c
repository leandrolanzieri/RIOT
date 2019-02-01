/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief       Test application for the AD7746 capacitance-to-digital
 *              converter with temperature sensor.
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include <stdio.h>

#include "xtimer.h"
#include "timex.h"
#include "ad7746.h"
#include "ad7746_params.h"

#define SLEEP       (500 * US_PER_MS)

static ad7746_t dev;

int main(void)
{
    uint32_t data;
    ad7746_vt_mode_t vt_mode = AD7746_VT_MD_TEMP;

    puts("AD746 capacitance to digital driver test application\n");
    printf("Initializing AD7746 at I2C_DEV(%i)... ",
           ad7746_params->i2c);

    if (ad7746_init(&dev, ad7746_params) == AD7746_OK) {
        puts("[OK]\n");
    }
    else {
        puts("[Failed]");
        return -1;
    }

    ad7746_set_vt_channel_mode(&dev, vt_mode);

    while (1) {
        int res;
        puts("=========================");
        puts("        Measuring");
        puts("=========================");
        res = ad7746_read_raw_cap_ch(&dev, &data);
        if ( res == AD7746_OK) {
            printf("Raw analog value: %"PRIu32"\n", data);
            printf("Capacitance %d fF\n", ad7746_raw_to_capacitance(data));
        }
        else {
            printf("Error reading data. err: %d\n", res);
        }

        res = ad7746_read_raw_vt_ch(&dev, &data);
        if ( res == AD7746_OK) {
            printf("Raw analog value: %"PRIu32"\n", data);
            if (vt_mode == AD7746_VT_MD_TEMP) {
                printf("Temperature: %d C\n", ad7746_raw_to_temperature(data));
            }
            else {
                printf("Voltage: %d mV\n", ad7746_raw_to_voltage(data));
            }
        }
        else {
            printf("Error reading data. err: %d\n", res);
        }

        vt_mode = (vt_mode == AD7746_VT_MD_TEMP) ? AD7746_VT_MD_VDD :
                                                   AD7746_VT_MD_TEMP;
        ad7746_set_vt_channel_mode(&dev, vt_mode);
        puts("");
        xtimer_usleep(SLEEP);
    }

    return 0;
}
