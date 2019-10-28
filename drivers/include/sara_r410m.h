/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_sara_r410m SARA-R410M LTE Cat M1/NB1 module driver
 * @ingroup     drivers_netdev
 * @brief       High-level driver for the SARA-R410M LTE Cat M1/NB1 modules
 * @{
 *
 * @file
 * @brief       Driver API for SARA-R410M LTE Cat M1/NB1 modules
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */

#ifndef SARA_R410M_H
#define SARA_R410M_H

#include "mutex.h"
#include "periph/gpio.h"
#include "at.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Types of supported sockets
 */
typedef enum sara_r410m_socket_type {
    SOCK_UDP = 17,  /**< UDP socket */
    SOCK_TCP = 6    /**< TCP socket */
} sara_r410m_socket_type_t;

/**
 * @brief Socket descriptor
 */
typedef struct sara_r410m_socket {
    sara_r410m_socket_type_t type;  /**< socket type */
    int id;                         /**< socket ID number (returned when created) */
} sara_r410m_socket_t;

/**
 * @brief SARA-R410M driver configuration parameters
 */
typedef struct sara_r410m_params {
    uart_t uart;            /**< UART device */
    uint32_t baudrate;      /**< UART baudrate */
    gpio_t reset_pin;       /**< pin to reset module */
    gpio_t pwr_on_pin;      /**< PWR_ON pin */
} sara_r410m_params_t;

/**
 * @brief SARA-R410M device descriptor
 */
typedef struct sara_r410m {
    at_dev_t at;                    /**< at device descriptor */
    sara_r410m_params_t params;     /**< driver configuration parameters */
} sara_r410m_t;

/**
 * @brief Possible device status
 */
typedef enum sara_r410m_status {
    STATUS_MINIMUM = 0,              /**< Minimum functionality */
    STATUS_ACTIVE = 1,               /**< Full functionality */
    STATUS_AIRPLANE_MODE = 4,        /**< Disable TX and RX */
    STATUS_SIM_TOOLKIT_ENABLE = 6,   /**< Enables SIM-toolkit interface */
    STATUS_SIM_TOOLKIT_DISABLE = 7,  /**< Disables SIM-toolkit interface */
    STATUS_SIM_TOOLKIT_RAW = 9,      /**< Enables SIM-toolkit interface in RAW mode */
    STATUS_SILENT_NO_SIM_RESET = 15, /**< Silent reset (no SIM card reset) */
    STATUS_SILENT_SIM_RESET = 16,    /**< Silent reset (with SIM card reset) */
    STATUS_MINIMUM_NO_SIM = 19,      /**< Disable TX, RX and SIM card */
    STATUS_HALT = 127                /**< Deep low power */
} sara_r410m_status_t;

/**
 * @brief Return codes
 */
enum sara_r410m_ret {
    SARA_R410M_ERR = -1,    /**< error */
    SARA_R410M_OK  = 0,     /**< success */
};

/**
 * @brief Initializes a SARA-R410M module with the given parameters @p params
 *
 * @param[in out] dev       device descriptor
 * @param[in] params        driver parameters
 *
 * @return SARA_R410M_OK    on success
 * @return SARA_R410M_ERR   on error
 */
int sara_r410m_init(sara_r410m_t *dev, const sara_r410m_params_t *params);

/**
 * @brief Checks if a SARA-R410M module is present on the bus.
 * 
 * @param[in] dev           device descriptor
 *
 * @return true     if device is present
 * @return false    otherwise
 */
bool sara_r410m_is_present(sara_r410m_t *dev);

/**
 * @brief Powers off the device. If a
 *        @ref sara_r41m_params_t::power_pin "power pin" has been defined, it
 *        is set. The AT device is turned off.
 *
 * @param[in] dev           device descriptor
 */
void sara_r410m_deinit(sara_r410m_t *dev);

/**
 * @brief Sets the band to be used
 *
 * @param[in] dev            device descriptor
 * @param[in] band           band to set
 * @return SARA_R410M_OK    on success
 * @return SARA_R410M_ERR   on error
 */
int sara_r410m_set_band(sara_r410m_t *dev, uint16_t band);

/**
 * @brief Sets the Access Point Name.
 * 
 * @param dev               device descriptor
 * @param apn               APN string
 *
 * @return SARA_R410M_OK    on success
 * @return SARA_R410M_ERR   on error
 */
int sara_r410m_set_apn(sara_r410m_t *dev, const char *apn);

/**
 * @brief Unlocks the SIM card. If a PIN is given it will use it, if not it
 *        assumes no PIN is needed.
 *
 * @param[in] dev           device descriptor
 * @param[in] pin           PIN to unlock SIM card. Can be NULL.
 *
 * @return SARA_R410M_OK    on success
 * @return SARA_R410M_ERR   on error 
 */
int sara_r410m_set_pin(sara_r410m_t *dev, const char *pin);

/**
 * @brief Issues a power off command to the device.
 *
 * @param[in] dev           device descriptor
 *
 * @return SARA_R410M_OK    on success
 * @return SARA_R410M_ERR   on error
 */
int sara_r410m_power_off(sara_r410m_t *dev);

/**
 * @brief Sets the status of the power saving configuration.
 *
 * @param[in] dev           device descriptor
 * @param[in] status        1 = On, 0 = Off
 *
 * @return SARA_R410M_OK    on success
 * @return SARA_R410M_ERR   on error
 */
int sara_r410m_power_saving_set(sara_r410m_t *dev, int status);

/**
 * @brief   Sets the device power status
 *
 * @param[in] dev           device descriptor
 * @param[in] status        status to change to
 *
 * @return 0 on success
 * @return <0 otherwise
 */
int sara_r410m_set_status(sara_r410m_t *dev, sara_r410m_status_t status);

/**
 * @brief   Register to a given network @p operator. The operator string should
 * be given in numeric format, and be 5 or 6 characters, null-terminated.
 *
 * @note This command can take up to @ref SARA_R410M_EXT_TIMEOUT us to return.
 *
 * @param[in] dev           device descriptor
 * @param[in] operator      operator number string
 *
 * @return 0 on success
 * @return <0 otherwise
 */
int sara_r410m_register(sara_r410m_t *dev, char *operator);

/**
 * @brief   Creates a socket of the given type.
 *
 * @note For now only UDP sockets are created.
 *
 * @param[in] dev                device descriptor
 * @param[in, out] socket        socket descriptor
 * @param[in] local_port         port to open the socket at
 *
 * @return 0 on success
 * @return -ETIMEDOUT if no response
 * @return -EINVAL if the socket could not be created
 */
int sara_r410m_socket_create(sara_r410m_t *dev, sara_r410m_socket_t *socket,
                             uint16_t local_port);

/**
 * @brief   Closes a given @p socket.
 *
 * @param[in] dev               device descriptor
 * @param[in] socket            socket to close
 *
 * @return 0 on success
 * @return <0 otherwise
 */
int sara_r410m_socket_close(sara_r410m_t *dev, sara_r410m_socket_t *socket);

/**
 * @brief   Sends @p data through a given @p socket. @p addr has to be a
 * null-terminated string of the address, it can be IPV4 or IPV6.
 *
 * @param[in] dev               device descriptor
 * @param[in] socket            socket to send the data through
 * @param[in] addr              IP address to send the data to
 * @param[in] port              port to send the data to
 * @param[in] data              data to send
 * @param[in] data_len          length of @p data
 *
 * @return @ref SARA_R410M_OK on success
 * @return -ETIMEDOUT on timeout of any of the commands
 * @return -EINVAL on not implemented socket type
 */
int sara_r410m_socket_send(sara_r410m_t *dev, sara_r410m_socket_t *socket,
                           char *addr, uint16_t port, char *data,
                           int data_len);

#ifdef __cplusplus
}
#endif

#endif /* SARA_R410M_H */
/** @} */
