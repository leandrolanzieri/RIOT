/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @ingroup     sys_auto_init_saul
 * @{
 *
 * @file
 * @brief       Auto initialization of AD7746
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */

#ifdef MODULE_AD7746

#include "assert.h"
#include "log.h"

#include "saul_reg.h"
#include "ad7746.h"
#include "ad7746_params.h"

/**
 * @brief   Define the number of configured sensors
 */
#define AD7746_NUM   (sizeof(ad7746_params) / sizeof(ad7746_params[0]))

/**
 * @brief   Allocate memory for the device descriptors
 */
static ad7746_t ad7746_devs[AD7746_NUM];

/**
 * @brief   Memory for the SAUL registry entries
 */
static saul_reg_t saul_entries[AD7746_NUM];

/**
 * @brief   Define the number of saul info
 */
#define AD7746_INFO_NUM (sizeof(ad7746_saul_info) / sizeof(ad7746_saul_info[0]))

/**
 * @brief   Reference the driver struct
 */
extern saul_driver_t ad7746_saul_driver;

void auto_init_ad7746(void)
{
    assert(AD7746_INFO_NUM == AD7746_NUM);

    for (unsigned i = 0; i < AD7746_NUM; i++) {
        LOG_DEBUG("[auto_init_saul] initializing ad7746 #%d\n", i);
        if (ad7746_init(&ad7746_devs[i], &ad7746_params[i]) < 0) {
            LOG_ERROR("[auto_init_saul] error initializing ad7746 #%d\n", i);
            continue;
        }

        saul_entries[i].dev = &(ad7746_devs[i]);
        saul_entries[i].name = ad7746_saul_info[i].name;
        saul_entries[i].driver = &ad7746_saul_driver;
        saul_reg_add(&(saul_entries[i]));
    }
}

#else
typedef int dont_be_pedantic;
#endif /* MODULE_AD7746 */
