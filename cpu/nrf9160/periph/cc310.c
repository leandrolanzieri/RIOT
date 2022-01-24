/*
 * Copyright (c) 2021 HAW Hamburg
 *
 */

#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kernel_defines.h"

#include "nrf_cc3xx_platform.h"
#include "nrf_cc3xx_platform_ctr_drbg.h"

#define ENABLE_DEBUG 1
#include "debug.h"


unsigned long gCcRegBase = 0;

int cc310_init(void)
{
    int res = 0;

    if (IS_USED(CC310_INTERRUPT)) {
        /* Enable interrupts */
        NVIC_EnableIRQ(CRYPTOCELL_IRQn);
    }

    // NRF_CRYPTOCELL_S->ENABLE=1;

    /* Set the RTOS abort APIs */
    nrf_cc3xx_platform_abort_init();

    /* Set the RTOS mutex APIs */
    nrf_cc3xx_platform_mutex_init();

    /* Initialize the cc3xx HW with RNG support */
    // res = nrf_cc3xx_platform_init_hmac_drbg();
    // res = nrf_cc3xx_platform_init_no_rng();
    res = nrf_cc3xx_platform_init();
    if (res)
    {
        DEBUG("Could not initialize cc310 platform (%x)\n", -res);
    }
    else
    {
        DEBUG("Initialized cc310 platform\n");
    }

    return res;
}

#if IS_USED(CC310_INTERRUPT)
void isr_cryptocell(void)
{
    CRYPTOCELL_IRQHandler();
}
#endif
