/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Hello World application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include "cc310.h"
#include "board.h"
#include "od.h"

#include "nrf_cc3xx_platform.h"
#include "nrf_cc3xx_platform_entropy.h"
#include "nrf_cc3xx_platform_kmu.h"

#define ENTROPY_LENGTH  16
uint8_t entropy_buffer[ENTROPY_LENGTH] = { 0 };

#define KEY_SLOT 2

nrf_cc3xx_platform_key_buff_t key_buff = {
    .buff_128 = {
        '1', '2', '3', '4', '5', '6', '7', '8',
        '9', '0', '1', '2', '3', '4', '5', '6'
    }
};

uint8_t plain_text[] = "thisisriot";

int main(void)
{
    LED0_ON;
    puts("Hello World!");

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);

    int res = cc310_init();
    if (!res) {
        puts("Success initializing CC310");
        LED1_ON;
    }
    else {
        puts("ERROR initializing CC310");
        LED0_OFF;
        return 1;
    }

    if (nrf_cc3xx_platform_rng_is_initialized()) {
        puts("RNG enabled");
        LED2_ON;
    }
    else {
        puts("RNG not enabled");
        LED0_OFF;
        LED1_OFF;
    }

    size_t entropy_length = 0;
    res = nrf_cc3xx_platform_entropy_get(entropy_buffer, ENTROPY_LENGTH, &entropy_length);
    if (!res) {
        puts("Success getting entropy");
        printf("Got %d bytes\n", entropy_length);
        od_hex_dump(entropy_buffer, ENTROPY_LENGTH, 0);
    }
    else {
        puts("ERROR getting entropy");
    }

    puts("Testing key management unit (KMU)");

    /* we test with AES-128 */
    res = nrf_cc3xx_platform_kmu_write_key(KEY_SLOT, NRF_CC3XX_PLATFORM_KEY_TYPE_AES_128_BIT, key_buff);
    if (NRF_CC3XX_PLATFORM_SUCCESS == res) {
        puts("Success writing key");
        LED3_ON;
    }
    else {
        puts("ERROR writing key");
    }

    return 0;
}
