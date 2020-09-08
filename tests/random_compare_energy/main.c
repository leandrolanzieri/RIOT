/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief       Low-level random number generator driver test
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "xtimer.h"
#include "random.h"
#include "periph/hwrng.h"

#include "periph/gpio.h"

/* BOARDS */
#ifdef BOARD_NUCLEO_F410RB
#define PIN0 GPIO_PIN(1,  5)
#define PIN1 GPIO_PIN(1,  3)
#endif

#ifdef BOARD_SAMR21_XPRO
#define PIN0 GPIO_PIN(0, 16)
#define PIN1 GPIO_PIN(0, 17)
#endif

#ifdef BOARD_REMOTE_REVA
#define PIN0 GPIO_PIN(2, 3)
#define PIN1 GPIO_PIN(2, 4)
#endif

#ifdef BOARD_PBA_D_01_KW2X
#define PIN0 GPIO_PIN(3, 1)
#define PIN1 GPIO_PIN(2, 4)
#endif

#ifdef BOARD_NRF52840DK
#define PIN0 GPIO_PIN(0, 27)
#define PIN1 GPIO_PIN(0, 26)
#endif

#ifdef ATRADIO_RNG
#include "at86rf2xx.h"
#include "at86rf2xx_internal.h"
#include "at86rf2xx_params.h"
#include "net/netdev.h"
#endif

#if TINYCRYPT_CTR
#include "tinycrypt/ctr_prng.h"
char * entropyString = "ce50f33da5d4c1d3d4004eb35244b7f2cd7f2e5076fbf6780a7ff634b249a5fc";
#define TINYCRYPT_ENTROPY_LEN 32
TCCtrPrng_t tinycrypt_ctx;
#endif

#if ATA
#include "atca.h"
#include "atca_params.h"
void call_io(void){}
#endif


#ifndef NUM_RAND_INTEGERS
#define NUM_RAND_INTEGERS   (1)
#endif

#ifndef NUM_RAND_BYTES
#define NUM_RAND_BYTES      (NUM_RAND_INTEGERS * 4)
#endif

#ifndef NUM_ITERATIONS
#define NUM_ITERATIONS      (10000)
#endif


#ifdef ATRADIO_RNG
static void _event_cb(netdev_t *dev, netdev_event_t event)
{
    (void)dev;
    (void)event;
}
#endif

int main(void)
{
    LED0_OFF;
    puts("START");
    printf("NUM_RAND_INTEGERS: %i\n", NUM_RAND_INTEGERS);
    printf("NUM_RAND_BYTES: %i\n", NUM_RAND_BYTES);
    printf("NUM_ITERATIONS: %i\n", NUM_ITERATIONS);
#ifdef ATRADIO_RNG
    puts("ATRADIO_RNG");
    uint8_t buf[NUM_RAND_BYTES];
#elif defined (MODULE_PERIPH_HWRNG)
    puts("HWRNG");
    uint8_t buf[NUM_RAND_BYTES];
#elif defined (MODULE_RANDOM)
    puts("PRNG");
    uint32_t buf[NUM_RAND_INTEGERS];
#elif defined SLEEP_ONLY
    printf("SLEEP ONLY: %i us\n", SLEEP_ONLY);
#endif

    gpio_init(PIN0, GPIO_OUT);
    gpio_init(PIN1, GPIO_OUT);
    // gpio_clear(PIN0);
    gpio_set(PIN0);
    gpio_clear(PIN1);
    // gpio_set(PIN1);

    xtimer_sleep(5);

#ifdef ATRADIO_RNG
    at86rf2xx_t at86rf2xx_dev;
    at86rf2xx_setup(&at86rf2xx_dev, &at86rf2xx_params[0]);
    netdev_t *dev = (netdev_t *)(&at86rf2xx_dev);
    dev->event_callback = _event_cb;
    dev->context = NULL;
    dev->driver->init(dev);
#elif defined (TINYCRYPT_CTR)
    uint8_t dummy[sizeof(int)];
    int ret = tc_ctr_prng_init(&tinycrypt_ctx, (uint8_t *)entropyString, TINYCRYPT_ENTROPY_LEN, NULL, 0);
    printf("tc_ctr_prng_init: %i\n",ret);
    tc_ctr_prng_generate(&tinycrypt_ctx, NULL, 0, dummy, sizeof(int));
#endif

        /* create random numbers */
#if (NUM_ITERATIONS < 0)
    for(;;) {
#else
    for (int i=0; i < NUM_ITERATIONS; i++) {
#endif
        // start trigger
        // puts("1");
        gpio_clear(PIN0);
        // gpio_set(PIN0);

        /* zero out buffer */
        // memset(buf, 0, sizeof(buf));
#ifdef ATRADIO_RNG
        at86rf2xx_get_random(&at86rf2xx_dev, (uint8_t *)&buf, sizeof(buf));
#elif defined (TINYCRYPT_CTR)
        uint8_t buf[NUM_RAND_BYTES];
        tc_ctr_prng_generate(&tinycrypt_ctx, NULL, 0, buf, NUM_RAND_BYTES);
#elif defined (ATA)
        uint8_t randout[32];
        atcab_random(randout);
#elif defined (MODULE_PERIPH_HWRNG)
        hwrng_read(buf, sizeof(buf));
#elif defined (MODULE_RANDOM)
        for(int j=0; j < NUM_RAND_INTEGERS; j++) {
            buf[j] = random_uint32();
        }
        (void)buf;
#elif defined SLEEP_ONLY
        xtimer_usleep(SLEEP_ONLY);
#endif
        // end trigger
        gpio_set(PIN1);
        // gpio_clear(PIN1);
        // puts("2");

        // reset pins and wait time to read buffer
        // 100ms each seems to work of for < 5ms exec time
        // was 750ms each with long ATE ext crypo chip
        xtimer_usleep(100 * 1000);
        // gpio_clear(PIN0);
        gpio_set(PIN0);
        xtimer_usleep(100 * 1000);
        gpio_clear(PIN1);
        // gpio_set(PIN1);
        xtimer_usleep(100 * 1000);
        int dummy, dummy2;
        for(dummy = 0; dummy < 10000; dummy++) {
            __asm__("nop");
        }
        (void)dummy2;
    }

    puts("END");
    LED0_ON;
    return 0;
}
