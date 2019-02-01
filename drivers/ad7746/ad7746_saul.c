/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ad7746
 * @{
 *
 * @file
 * @brief       AD7746 adaption to the RIOT actuator/sensor interface
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */

#include <string.h>
#include <stdio.h>

#include "saul.h"
#include "ad7746.h"

static int _read(const void *dev, phydat_t *res)
{
    uint32_t raw;
    if (ad7746_read_raw_ch((const ad7746_t *)dev, AD7746_READ_CAP_CH, &raw)) {
        return -ECANCELED;
    }

    res->val[0] = ad7746_raw_to_capacitance(raw);
    res->unit = UNIT_F;
    res->scale = -15;

    return 1;
}

const saul_driver_t ad7746_saul_driver = {
    .read = _read,
    .write = saul_notsup,
    .type = SAUL_SENSE_CAP,
};
