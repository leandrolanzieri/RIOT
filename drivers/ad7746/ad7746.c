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
 * @brief       AD7746 Capacitance-to-digital converter with temperature
 *              sensor driver
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include "ad7746.h"
#include "ad7746_params.h"
#include "ad7746_internal.h"

#include "periph/i2c.h"
#include "periph/gpio.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#include "xtimer.h"

#define I2C (dev->params.i2c)
#define ADDR (dev->params.addr)

#define CONF_TEST_VALUE (1 << AD7746_CONFIGURATION_VTF1_BIT)

int ad7746_init(ad7746_t *dev, const ad7746_params_t *params)
{
    int status;
    int res = AD7746_NOI2C;

    assert(dev && params);
    dev->params = *params;

    i2c_acquire(I2C);
    uint8_t reg = 0;

    /* Test communication write and read configuration register */
    status = i2c_write_reg(I2C, ADDR, AD7746_REG_CONFIGURATION, CONF_TEST_VALUE,
                           0);
    status += i2c_read_reg(I2C, ADDR, AD7746_REG_CONFIGURATION, &reg, 0);

    if (status < 0 || reg != CONF_TEST_VALUE) {
        DEBUG("[ad7746] init - error: unable to communicate with the device "
              "(reg=%x)\n", reg);
        res = AD7746_NODEV;
        goto release;
    }

    /* Enable capacitive channel */
    reg = (1 << AD7746_CAP_SETUP_CAPEN_BIT);
    if (i2c_write_reg(I2C, ADDR, AD7746_REG_CAP_SETUP, reg, 0)) {
        DEBUG("[ad7746] init - error: unable to enable capacitive channel\n");
        goto release;
    }

    if (dev->params.vt_mode != AD7746_VT_MD_DIS) {
        /* Enable voltage / temperature channel and set mode */
        reg = (1 << AD7746_VT_SETUP_VTEN_BIT) |
              (dev->params.vt_mode << AD7746_VT_SETUP_VTMD0_BIT);
        if (i2c_write_reg(I2C, ADDR, AD7746_REG_VT_SETUP, reg, 0)) {
            DEBUG("[ad7746] init - error: unable to enable the v/t channel\n");
            goto release;
        }
    }

    /* Set EXC sources */
    reg = (dev->params.exc_config << AD7746_EXC_SETUP_INV_EXCA_BIT);
    if (i2c_write_reg(I2C, ADDR, AD7746_REG_EXC_SETUP, reg, 0)) {
        DEBUG("[ad7746] init - error: unable to set EXC outputs\n");
        goto release;
    }

    /* Set DAC A capacitance value */
    if (dev->params.dac_a_cap) {
        assert(dev->params.dac_a_cap <= AD7746_DAC_MAX);
        reg = (1 << AD7746_DACAEN_BIT) | dev->params.dac_a_cap;
        if (i2c_write_reg(I2C, ADDR, AD7746_REG_CAP_DAC_A, reg, 0)) {
            DEBUG("[ad7746] init - error: unable to set DAC A\n");
            goto release;
        }
    }

    /* Set DAC B capacitance value */
    if (dev->params.dac_b_cap) {
        assert(dev->params.dac_b_cap <= AD7746_DAC_MAX);
        reg = (1 << AD7746_DACBEN_BIT) | dev->params.dac_b_cap;
        if (i2c_write_reg(I2C, ADDR, AD7746_REG_CAP_DAC_B, reg, 0)) {
            DEBUG("[ad7746] init - error: unable to set DAC B\n");
            goto release;
        }
    }

    /* Set to continuous mode and configured sample rates */
    reg = (1 << AD7746_CONFIGURATION_MD0_BIT) |
          (dev->params.cap_sample_rate << AD7746_CONFIGURATION_CAPF0_BIT) |
          (dev->params.vt_sample_rate << AD7746_CONFIGURATION_VTF0_BIT);
    if (i2c_write_reg(I2C, ADDR, AD7746_REG_CONFIGURATION, reg, 0)) {
        DEBUG("[ad7746] init - error: unable to set mode and SR\n");
        goto release;
    }

    res = AD7746_OK;

release:
    i2c_release(I2C);
    return res;
}

int ad7746_set_vt_channel_mode(const ad7746_t *dev, ad7746_vt_mode_t mode)
{
    uint8_t reg;
    int res = AD7746_NOI2C;

    assert(dev);
    i2c_acquire(I2C);
    if (i2c_read_reg(I2C, ADDR, AD7746_REG_VT_SETUP, &reg, 0)) {
        goto release;
    }

    if (mode == AD7746_VT_MD_DIS) {
        reg &= ~(1 << AD7746_VT_SETUP_VTEN_BIT);
    }
    else {
        /* Enable voltage / temperature channel and set mode */
        reg &= ~((1 << AD7746_VT_SETUP_VTMD0_BIT) |
                 (1 << AD7746_VT_SETUP_VTMD1_BIT));
        reg |= (1 << AD7746_VT_SETUP_VTEN_BIT) |
               (mode << AD7746_VT_SETUP_VTMD0_BIT);
    }

    if (i2c_write_reg(I2C, ADDR, AD7746_REG_VT_SETUP, reg, 0)) {
        DEBUG("[ad7746] set_vt_ch - error: unable to set v/t channel mode\n");
        goto release;
    }

release:
    i2c_release(I2C);
    return res;
}

int ad7746_read_raw_ch(const ad7746_t *dev, uint8_t ch, uint32_t *raw)
{
    uint8_t buf[3];
    uint8_t res = AD7746_NOI2C;
    uint16_t reg = (ch == AD7746_READ_CAP_CH) ? AD7746_REG_CAP_DATA_H :
                                                AD7746_REG_VT_DATA_H;

    assert(dev);
    assert((ch == AD7746_READ_CAP_CH) || (ch == AD7746_READ_VT_CH));
    i2c_acquire(I2C);

    if (i2c_read_reg(I2C, ADDR, AD7746_REG_STATUS, buf, 0)) {
        goto release;
    }

    /* check if data is available from the requested channel */
    if (buf[0] & (1 << ch)) {
        res = AD7746_NODATA;
            DEBUG("[ad7746] read_raw: No data available\n");
        goto release;
    }

    if (i2c_read_regs(I2C, ADDR, reg, buf, 3, 0)) {
        goto release;
    }

    *raw = (((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) |
            ((uint32_t)buf[2]));
    res = AD7746_OK;

release:
    i2c_release(I2C);
    return res;
}

int ad7746_raw_to_capacitance(uint32_t raw)
{
    int32_t value = (raw - AD7746_ZERO_SCALE_CODE) >> 11;
    return (int)value;
}

int ad7746_raw_to_temperature(uint32_t raw)
{
    int value = (raw >> 11) - 4096;
    return value;
}

int ad7746_raw_to_voltage(uint32_t raw)
{
    int64_t value = (raw - AD7746_ZERO_SCALE_CODE);
    value *= AD7746_INTERNAL_VREF;
    value >>= 23;
    return (int)value;
}

int ad7746_set_cap_ch_input(const ad7746_t *dev, uint8_t input)
{
    uint8_t reg;
    int res = AD7746_NOI2C;

    assert(dev);
    i2c_acquire(I2C);

    if (i2c_read_reg(I2C, ADDR, AD7746_REG_CAP_SETUP, &reg, 0)) {
        goto release;
    }

    if (input) {
        reg |= (1 << AD7746_CAP_SETUP_CIN2_BIT);
    }
    else {
        reg &= ~(1 << AD7746_CAP_SETUP_CIN2_BIT);
    }

    if (i2c_write_reg(I2C, ADDR, AD7746_REG_CAP_SETUP, reg, 0)) {
        goto release;
    }

    res = AD7746_OK;

release:
    i2c_release(I2C);
    return res;
}

int ad7746_set_cap_sr(const ad7746_t *dev, ad7746_cap_sample_rate_t sr)
{
    uint8_t reg;
    int res = AD7746_NOI2C;

    assert(dev);
    i2c_acquire(I2C);

    if (i2c_read_reg(I2C, ADDR, AD7746_REG_CONFIGURATION, &reg, 0)) {
        goto release;
    }

    reg &= ~(7 << AD7746_CONFIGURATION_CAPF0_BIT);
    reg |= (sr << AD7746_CONFIGURATION_CAPF0_BIT);

    if (i2c_write_reg(I2C, ADDR, AD7746_REG_CONFIGURATION, reg, 0)) {
        goto release;
    }

    res = AD7746_OK;

release:
    i2c_release(I2C);
    return res;
}

int ad7746_set_vt_sr(const ad7746_t *dev, ad7746_vt_sample_rate_t sr)
{
    uint8_t reg;
    int res = AD7746_NOI2C;

    assert(dev);
    i2c_acquire(I2C);

    if (i2c_read_reg(I2C, ADDR, AD7746_REG_CONFIGURATION, &reg, 0)) {
        goto release;
    }

    reg &= ~(3 << AD7746_CONFIGURATION_VTF0_BIT);
    reg |= (sr << AD7746_CONFIGURATION_VTF0_BIT);

    if (i2c_write_reg(I2C, ADDR, AD7746_REG_CONFIGURATION, reg, 0)) {
        goto release;
    }

    res = AD7746_OK;

release:
    i2c_release(I2C);
    return res;
}
