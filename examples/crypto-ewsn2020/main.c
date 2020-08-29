/*
 * Copyright (C) 2020 HAW Hamburg
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
 * @brief       Application to measure and compare crypto runtime
 *
 * @author      Lena Boeckmann <lena.boeckmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "kernel_defines.h"

#ifdef ARM_CRYPTOCELL
#include "armcc_setup.h"
#endif

#include "crypto_runtime.h"

#include "periph/gpio.h"
#include "periph_conf.h"

#if (IS_ACTIVE(MODULE_LIB_MMCAU)) && (defined(BOARD_PBA_D_01_KW2X))
    gpio_t active_gpio = GPIO_PIN(2,5);
#endif /*MODULE_LIB_MMCAU*/

int main(void)
{
    gpio_init(active_gpio, GPIO_OUT);

    sha1_test(active_gpio);
    sha256_test(active_gpio);
    aes_cbc_test(active_gpio);
    aes_ctr_test(active_gpio);
    hmac_sha256_test(active_gpio);

    return 1;
}