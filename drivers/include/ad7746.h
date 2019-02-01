/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup   drivers_ad7746 AD7746 Capacitance-to-digital converter driver
 * @ingroup    drivers_sensors
 * @ingroup    drivers_saul
 * @brief      I2C Capacitance-to-digital converter with temperature sensor
 *
 * This driver provides @ref drivers_saul capabilities.
 * @{
 *
 * @file
 * @brief      AD7746 Capacitance-to-digital converter with temperature
 *             sensor driver
 *
 * @author     Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */

#ifndef AD7746_H
#define AD7746_H

#ifdef __cplusplus
extern "C" {
#endif

#include "periph/i2c.h"
#include "periph/gpio.h"

/**
 * @brief  AD7746 default address
 */
#ifndef AD7746_I2C_ADDRESS
#define AD7746_I2C_ADDRESS  (0x48)
#endif

/**
 * @brief 0fF capacitance code
 */
#define AD7746_ZERO_SCALE_CODE  (0x800000LL)

/**
 * @brief Interval voltage reference expressed in mV
 */
#define AD7746_INTERNAL_VREF    (1170)

/**
 * @brief Maximum value that can be configured into the DACs.
 */
#define AD7746_DAC_MAX          (0x7F)

/**
 * @brief Channel numbers for reading
 */
enum {
    AD7746_READ_CAP_CH = 0, /**< read capacitive channel */
    AD7746_READ_VT_CH  = 1 /**< read voltage / temperature channel */
};

/**
 * @brief   Named return values
 */
enum {
    AD7746_OK          =  0,       /**< everything was fine */
    AD7746_NOI2C       = -1,       /**< I2C communication failed */
    AD7746_NODEV       = -2,       /**< no AD7746 device found on the bus */
    AD7746_NODATA      = -3        /**< no data available */
};

/**
 * @brief Voltage / Temperature channel sample rates
 */
typedef enum {
    AD7746_VT_SR_498 = 0, /**< 49.8 Hz */
    AD7746_VT_SR_312 = 1, /**< 31.2 Hz */
    AD7746_VT_SR_161 = 2, /**< 16.1 Hz */
    AD7746_VT_SR_082 = 3  /**<  8.2 Hz */
} ad7746_vt_sample_rate_t;

/**
 * @brief Voltage / Temperature channel modes
 */
typedef enum {
    AD7746_VT_MD_DIS   = -1, /**< channel disabled */
    AD7746_VT_MD_TEMP  = 0,  /**< internal temperature sensor */
    AD7746_VT_MD_ETEMP = 1,  /**< external temperature sensor (see datasheet) */
    AD7746_VT_MD_VDD   = 2,  /**< Vdd voltage monitor */
    AD7746_VT_MD_VIN   = 3   /**< external voltage input (Vin) */
} ad7746_vt_mode_t;

/**
 * @brief Capacitive channel sample rate
 */
typedef enum {
    AD7746_CAP_SR_909 = 0, /**< 90.9 Hz */
    AD7746_CAP_SR_838 = 1, /**< 83.8 Hz */
    AD7746_CAP_SR_500 = 2, /**< 50.0 Hz */
    AD7746_CAP_SR_263 = 3, /**< 26.3 Hz */
    AD7746_CAP_SR_161 = 4, /**< 16.1 Hz */
    AD7746_CAP_SR_130 = 5, /**< 13.0 Hz */
    AD7746_CAP_SR_109 = 6, /**< 10.9 Hz */
    AD7746_CAP_SR_091 = 7  /**<  9.1 Hz */
} ad7746_cap_sample_rate_t;

/**
 * @brief Excitation signal outpus configuration
 */
typedef enum {
    AD7746_EXC_A = 0x06,   /**< enable only exc A output */
    AD7746_EXC_B = 0x09,   /**< enable only exc B output */
    AD7746_EXC_AB = 0x0A   /**< enable exc A and B outputs */
} ad7746_exc_config_t;

/**
 * @brief   AD7746 params
 */
typedef struct ad7746_params {
    i2c_t i2c;                                /**< i2c device */
    uint8_t addr;                             /**< i2c address */
    uint8_t dac_a_cap;                        /**< DAC A capacitance */
    uint8_t dac_b_cap;                        /**< DAC B capacitance */
    ad7746_exc_config_t exc_config;           /**< excitation signal config */
    ad7746_cap_sample_rate_t cap_sample_rate; /**< capacitance sample rate */
    ad7746_vt_sample_rate_t vt_sample_rate;   /**< voltage/temp sample rate */
    ad7746_vt_mode_t vt_mode;                 /**< mode of the voltage/temp ch */
} ad7746_params_t;


/**
 * @brief   AD7746 device descriptor
 */
typedef struct ad7746 {
    ad7746_params_t params; /**< device driver configuration */
} ad7746_t;

/**
 * @brief   Initializes an AD7746 device
 *
 * @param[in,out] dev  device descriptor
 * @param[in] params   device configuration
 *
 * @return AD7746_OK on success
 * @return AD7746_NODEV if no device is found on the bus
 * @return AD7746_NOI2C if other error occurs
 */
int ad7746_init(ad7746_t *dev, const ad7746_params_t *params);

/**
 * @brief   Reads a raw value either from the capacitance channel or from the
 *          voltage / temperature channel if available
 *
 * @param[in] dev   device descriptor
 * @param[in] ch    channel to read:
 *                      - AD7746_READ_CAP_CH: Capacitance
 *                      - AD7746_READ_VT_CH: Voltage / temperature
 * @param[out] raw  read value
 *
 * @return AD7746_OK on success
 * @return AD7746_NODATA if no data is available
 * @return AD7746_I2C on error getting a reponse
 */
int ad7746_read_raw_ch(const ad7746_t *dev, uint8_t ch, uint32_t *raw);

/**
 * @brief   Convenience function that reads a raw value from the capacitance
 *          channel
 *
 * @param[in] dev   device descriptor
 * @param[out] raw  read value
 *
 * @return AD7746_OK on success
 * @return AD7746_NODATA if no data is available
 * @return AD7746_I2C on error getting a reponse
 */
static inline int ad7746_read_raw_cap_ch(const ad7746_t *dev, uint32_t *raw)
{
    return ad7746_read_raw_ch(dev, AD7746_READ_CAP_CH, raw);
}

/**
 * @brief   Convenience function that reads a raw value from the voltage /
 *          temperature channel
 *
 * @param[in] dev   device descriptor
 * @param[out] raw  read value
 *
 * @return AD7746_OK on success
 * @return AD7746_NODATA if no data is available
 * @return AD7746_I2C on error getting a reponse
 */
static inline int ad7746_read_raw_vt_ch(const ad7746_t *dev, uint32_t *raw)
{
    return ad7746_read_raw_ch(dev, AD7746_READ_VT_CH, raw);
}

/**
 * @brief   Converts a raw code into a capacitance expressed in fF
 *
 * @param[in] raw   Raw code from the device
 *
 * @return capacitance in fF
 */
int ad7746_raw_to_capacitance(uint32_t raw);

/**
 * @brief   Converts a raw code into temperature expressed in celsius (C)
 *
 * @param[in] raw   Raw code from the device
 *
 * @return temperature in celsius
 */
int ad7746_raw_to_temperature(uint32_t raw);

/**
 * @brief   Converts a raw code into voltage expressed in mV
 *
 * @param[in] raw   Raw code from the device
 * @return voltage in mV
 *
 * @note Note when using the @ref AD7746_VT_MD_VDD mode, that the voltage from the
 *       VDD pin is internally attenuated by 6
 */
int ad7746_raw_to_voltage(uint32_t raw);

/**
 * @brief   Sets the current input for the capacitive measurement
 *
 * @param[in] dev      device descriptor
 * @param[in] input  selected input - 0 for CIN1, 1 for CIN2
 *
 * @return AD7746_OK on success
 * @return AD7746_NOI2C on error
 */
int ad7746_set_cap_ch_input(const ad7746_t *dev, uint8_t input);

/**
 * @brief   Sets the mode for the voltage / temperature channel
 *
 * @param[in] dev   device descriptor
 * @param[in] mode  mode to which the channel has to be set
 *
 * @return AD7746_OK on success
 * @return AD7746_NOI2C on error
 */
int ad7746_set_vt_channel_mode(const ad7746_t *dev, ad7746_vt_mode_t mode);

/**
 * @brief   Sets the sample rate for the voltage / temperature channel
 *
 * @param[in] dev   device descriptor
 * @param[in] sr    sample rate
 *
 * @return AD7746_OK on success
 * @return AD7746_NOI2C on error
 */
int ad7746_set_vt_sr(const ad7746_t *dev, ad7746_vt_sample_rate_t sr);

/**
 * @brief   Sets the sample rate for the capacitance channel
 *
 * @param[in] dev   device descriptor
 * @param[in] sr    sample rate
 *
 * @return AD7746_OK on success
 * @return AD7746_NOI2C on error
 */
int ad7746_set_cap_sr(const ad7746_t *dev, ad7746_cap_sample_rate_t sr);

#ifdef __cplusplus
}
#endif

#endif /* AD7746_H */
/** @} */
