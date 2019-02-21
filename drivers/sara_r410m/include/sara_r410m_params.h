/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_sara_r410m
 * @{
 *
 * @file
 * @brief       Default configuration parameters for SARA-R410M LTE Cat M1/NB1
 *              modules
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */

#ifndef SARA_R410M_PARAMS_H
#define SARA_R410M_PARAMS_H

#include "sara_r410m.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup   drivers_sara_r410m_nbiot_conf    SARA-R410M NB-IoT compile configurations
 * @ingroup    drivers_sara_r410m
 * @ingroup    config
 * @{
 */
/** @brief  Operators Access Point Name */
#ifndef SARA_R410M_NBIOT_APN
#define SARA_R410M_NBIOT_APN        "iot.1nce.net"
#endif

/** @brief  Operators code */
#ifndef SARA_R410M_NBIOT_OPERATOR
#define SARA_R410M_NBIOT_OPERATOR   "26201"
#endif

/** @brief  SIM card PIN (should be a string) */
#ifndef SARA_R410M_NBIOT_PIN
#define SARA_R410M_NBIOT_PIN        NULL
#endif

/** @brief  NB-IoT band number */
#ifndef SARA_R410M_NBIOT_BAND
#define SARA_R410M_NBIOT_BAND       (8)
#endif
/** @} */

/**
 * @defgroup    drivers_sara_r410m_conf     SARA-R410M driver compile configurations
 * @ingroup     drivers_sara_r410m
 * @ingroup     config
 * @{
 */
/** @brief UART device */
#ifndef SARA_R410M_PARAM_UART
#define SARA_R410M_PARAM_UART        UART_DEV(1)
#endif

/** @brief  UART baudrate */
#ifndef SARA_R410M_PARAM_BAUDRATE
#define SARA_R410M_PARAM_BAUDRATE    (115200)
#endif

/** @brief Reset pin */
#ifndef SARA_R410M_PARAM_RESET_PIN
#define SARA_R410M_PARAM_RESET_PIN   GPIO_PIN(PB, 14)
#endif

/** @brief VCC pin */
#ifndef SARA_R410M_PARAM_VCC_PIN
#define SARA_R410M_PARAM_VCC_PIN     GPIO_PIN(PA, 27)
#endif

/** @brief PWR_ON pin */
#ifndef SARA_R410M_PARAM_PWR_ON_PIN
#define SARA_R410M_PARAM_PWR_ON_PIN  GPIO_PIN(PB, 17)
#endif

#ifndef SARA_R410M_OPERATOR_MAX_LEN
#define SARA_R410M_OPERATOR_MAX_LEN  (6)
#endif

#ifndef SARA_R410M_AT_BUF_SIZE
#define SARA_R410M_AT_BUF_SIZE       (256)
#endif

#ifndef SARA_R410M_STD_TIMEOUT
#define SARA_R410M_STD_TIMEOUT       (10000000UL)
#endif

#ifndef SARA_R410M_EXT_TIMEOUT
#define SARA_R410M_EXT_TIMEOUT       (6 * SARA_R410M_STD_TIMEOUT)
#endif
/** @} */

#define SARA_R410M_NBIOT_CONFIG   { .apn = SARA_R410M_NBIOT_APN,            \
                                    .op = SARA_R410M_NBIOT_OPERATOR,        \
                                    .sim_pin = SARA_R410M_NBIOT_PIN,        \
                                    .band = SARA_R410M_NBIOT_BAND }

#define SARA_R410M_PARAMS   { .uart = SARA_R410M_PARAM_UART,                \
                              .baudrate = SARA_R410M_PARAM_BAUDRATE,        \
                              .reset_pin = SARA_R410M_PARAM_RESET_PIN,      \
                              .pwr_on_pin = SARA_R410M_PARAM_PWR_ON_PIN,    \
                              .vcc_pin = SARA_R410M_PARAM_VCC_PIN }

/**
 * @brief   SARA-R410M NB-IoT configurations
 *
 */
static const sara_r410m_nbiot_t sara_r410m_nbiot_configs[] = {
    SARA_R410M_NBIOT_CONFIG
};

/**
 * @brief   SARA-R410M driver configurations
 *
 */
static const sara_r410m_params_t sara_r410m_params[] = {
    SARA_R410M_PARAMS
};

#ifdef __cplusplus
}
#endif

#endif /* SARA_R410M_PARAMS_H */
