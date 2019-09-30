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
 * @brief       Driver implementation for SARA-R410M LTE Cat M1/NB1
 *              modules
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include "errno.h"

#include "xtimer.h"
#include "fmt.h"
#include "periph/gpio.h"
#include "sara_r410m_params.h"
#include "sara_r410m.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

static char buf[SARA_R410M_AT_BUF_SIZE];

static void _power_on_seq(sara_r410m_t *dev)
{
    /* set reset */
    if (dev->params.reset_pin != GPIO_UNDEF) {
        gpio_init(dev->params.reset_pin, GPIO_OUT);
        gpio_set(dev->params.reset_pin);
    }

    /* send PWR_ON pulse */
    if (dev->params.pwr_on_pin != GPIO_UNDEF) {
        gpio_init(dev->params.pwr_on_pin, GPIO_OUT);
        gpio_clear(dev->params.pwr_on_pin);
        xtimer_usleep(200 * US_PER_MS);
        gpio_init(dev->params.pwr_on_pin, GPIO_IN);
    }
}

int sara_r410m_init(sara_r410m_t *dev, const sara_r410m_params_t *params)
{
    DEBUG("Turning SARA R410M module.\n");
    dev->params = *params;

    at_dev_init(&dev->at, dev->params.uart, dev->params.baudrate, buf,
                sizeof(buf));

    _power_on_seq(dev);

    /* wait for device ready */
    while (!sara_r410m_is_present(dev)) {
        xtimer_usleep(100 * US_PER_MS);
    }

    /* Set radio technology to LTE Cat.NB1 */
    if (at_send_cmd_wait_ok(&dev->at, "AT+URAT=8",
                            SARA_R410M_STD_TIMEOUT)) {
        DEBUG("Could not set radio technology\n");
        goto error;
    }

    /* Set numeric values on error */
    if (at_send_cmd_wait_ok(&dev->at, "AT+CMEE=2",
                            SARA_R410M_STD_TIMEOUT)) {
        DEBUG("Could not set error configuration\n");
        goto error;
    }

    return SARA_R410M_OK;

error:
    DEBUG("[SARA R410M] Error configuring device\n");
    return SARA_R410M_ERR;
}

void sara_r410m_deinit(sara_r410m_t *dev)
{
    // if (dev->params.power_pin != GPIO_UNDEF) {
    //     gpio_set(dev->params.power_pin);
    //     gpio_init(dev->params.power_pin, GPIO_OUT);
    // }
    at_dev_poweroff(&dev->at);
}

bool sara_r410m_is_present(sara_r410m_t *dev)
{
    if (at_send_cmd_wait_ok(&dev->at, "AT", SARA_R410M_STD_TIMEOUT)) {
        DEBUG("Sara R410M is not present\n");
        return false;
    }
    DEBUG("SARA R410M is present\n");
    return true;
}

int sara_r410m_power_off(sara_r410m_t *dev)
{
    if (at_send_cmd_wait_ok(&dev->at, "AT+CPWROFF",
                            SARA_R410M_STD_TIMEOUT)) {
        DEBUG("[SARA R410M] Could not power off\n");
        return -ETIMEDOUT;
    }
    return SARA_R410M_OK;
}

int sara_r410m_register(sara_r410m_t *dev, char *operator)
{
    sara_r410m_set_status(dev, STATUS_ACTIVE);

    at_send_bytes(&dev->at, "AT+COPS=1,2,\"", sizeof("AT+COPS=1,2,\"") - 1);
    at_send_bytes(&dev->at, operator, strlen(operator));

    return at_send_cmd_wait_ok(&dev->at, "\"", SARA_R410M_EXT_TIMEOUT);
}

int sara_r410m_power_saving_set(sara_r410m_t *dev, int status)
{
    char _status[] = "0";
    _status[0] = status ? '4' : '0';
    DEBUG("[SARA R410M] Setting power saving %d\n", status);

    at_send_bytes(&dev->at, "AT+UPSV=", sizeof("AT+UPSV=") - 1);
    return at_send_cmd_wait_ok(&dev->at, _status, SARA_R410M_STD_TIMEOUT);
}

int sara_r410m_socket_create(sara_r410m_t *dev, sara_r410m_socket_t *socket,
                             uint16_t local_port)
{
    assert(dev && socket);
    char msg[] = "AT+USOCR=00,00000";
    char resp[32];
    sprintf(msg, "AT+USOCR=%d,%d", socket->type, local_port);

    if ((at_send_cmd_get_resp(&dev->at, msg, resp, sizeof(resp),
                              SARA_R410M_STD_TIMEOUT)) < 0) {
        DEBUG("[SARA R410M] Error: We got no response\n");
        return -ETIMEDOUT;
    }

    char *pos = strstr(resp, "USOCR: ");
    if (pos == NULL) {
        DEBUG("[SARA R410M] Error: Could not create a socket\n");
        return -EINVAL;
    }

    pos += sizeof("USOCR: ") - 1;
    socket->id = *pos - '0';
    DEBUG("[SARA R410M] Socket id: %d\n", socket->id);
    return 0;
}

int sara_r410m_socket_close(sara_r410m_t *dev, sara_r410m_socket_t *socket)
{
    char number[] = "0";

    number[0] = (socket->id + '0');
    at_send_bytes(&dev->at, "AT+USOCL=", sizeof("AT+USOCL=") - 1);
    return at_send_cmd_wait_ok(&dev->at, number, SARA_R410M_EXT_TIMEOUT);
}

int sara_r410m_socket_send(sara_r410m_t *dev, sara_r410m_socket_t *socket,
                           char *addr, uint16_t port, char *data, int data_len)
{
    char msg[256]; // TODO: Calculate size

    if (socket->type == SOCK_UDP) {
        DEBUG("[SARA R410M] Sending using UDP socket\n");

        at_drain(&dev->at);

        sprintf(msg, "AT+USOST=%d,\"%s\",%d,%d", socket->id, addr, port,
                data_len);
        at_send_cmd(&dev->at, msg, SARA_R410M_STD_TIMEOUT);

        if (at_expect_bytes(&dev->at, "@", SARA_R410M_STD_TIMEOUT)) {
            DEBUG("[SARA R410M] Error: no prompt to send\n");
            return -ETIMEDOUT;
        }

        sprintf(msg, "\r\n+USOST: %d,%d\r\n\r\nOK",socket->id, data_len);
        at_send_bytes(&dev->at, data, data_len);

        if (at_expect_bytes(&dev->at, data, SARA_R410M_STD_TIMEOUT)) {
            DEBUG("[SARA R410M] Error: no echo of data\n");
            return -ETIMEDOUT;
        }

        if (at_expect_bytes(&dev->at, msg, SARA_R410M_STD_TIMEOUT)) {
            DEBUG("[SARA R410M] Error: no OK after send\n");
            return -ETIMEDOUT;
        }
    }
    else {
        DEBUG("[SARA R410M] Error: TCP send not implemented\n");
        return -EINVAL;
    }

    return SARA_R410M_OK;
}

int sara_r410m_socket_receive(sara_r410m_t *dev, sara_r410m_socket_t *socket,
                              char *addr, size_t addr_len, uint16_t port,
                              char *data, size_t data_len)
{
    (void)addr_len;
    (void)port;
    assert(dev && socket && addr);

    char msg[SARA_R410M_AT_BUF_SIZE]; // TODO: Calculate size
    if (socket->type == SOCK_UDP) {
        DEBUG("[SARA R410M] Receiving from UDP socket\n");
        at_drain(&dev->at);
        /* if no data buffer specified just check the amount of bytes available */
        sprintf(msg, "AT+USORF=%d,%d", socket->id, data ? data_len : 0);
        at_send_cmd(&dev->at, msg, SARA_R410M_STD_TIMEOUT);
    }
    else {
        DEBUG("[SARA R410M] Error: TCP receive not implemented\n");
        return SARA_R410M_ERR;
    }
    return SARA_R410M_OK;
}
