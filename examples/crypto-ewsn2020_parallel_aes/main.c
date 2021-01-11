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
 * @brief       Application to showcase parallel AES + DMA accelerated
 *              encryption
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "crypto_runtime.h"
#include "periph/gpio.h"
#include "periph_conf.h"

int main(void)
{
    aes_ecb_parallel_test(&thread1_pin, &thread2_pin);
    return 1;
}
