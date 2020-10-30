/*
 * Copyright (C) 2020 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_efm32
 * @{
 *
 * @file
 * @brief       Implementation of some management functions for the efm32 Crypto module
 *
 * @author      Lena Boeckmann <lena.boeckmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdbool.h>
#include <stdio.h>
#include "crypto_util.h"
#include "mutex.h"

#if CPU_MODEL_EFM32PG12B500F1024GL125
static int acqu_count = 0;
static bool crypto_lock_initialized = false;
static mutex_t crypto_lock[CRYPTO_COUNT];
static const crypto_device_t crypto_devs[CRYPTO_COUNT] =
{
    #if defined(CRYPTO0)
        {
            CRYPTO0,
            cmuClock_CRYPTO0
        },
    #elif defined(CRYPTO)
        {
            CRYPTO,
            cmuClock_CRYPTO
        },
    #endif
    #if defined(CRYPTO1)
        {
            CRYPTO1,
            cmuClock_CRYPTO1
        }
    #endif
};

CRYPTO_TypeDef* crypto_acquire(void) {
    CRYPTO_TypeDef* dev = NULL;
    if (!crypto_lock_initialized) {
        /* initialize lock */
        for (int i = 0; i < CRYPTO_COUNT; i++) {
            mutex_init(&crypto_lock[i]);
        }
        crypto_lock_initialized = true;
    }

    int devno = acqu_count % CRYPTO_COUNT;
    mutex_lock(&crypto_lock[devno]);
    dev = crypto_devs[devno].dev;

    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(crypto_devs[devno].cmu, true);

    acqu_count++;
    return dev;
}

static int get_devno(CRYPTO_TypeDef* dev)
{
    if (CRYPTO_COUNT == 1) {
        return 0;
    }
    else if (CRYPTO_COUNT == 2 && dev == CRYPTO0) {
        return 0;
    }
    else if (CRYPTO_COUNT == 2 && dev == CRYPTO1) {
        return 1;
    }
    return -1;
}

void crypto_release(CRYPTO_TypeDef* dev)
{
    int devno = get_devno(dev);
    if (devno < 0) {
        return;
    }
    CMU_ClockEnable(crypto_devs[devno].cmu, false);

    acqu_count--;
    mutex_unlock(&crypto_lock[devno]);
}
#endif