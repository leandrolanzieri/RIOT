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
 * @brief       this is an ecdh test application
 *
 * @author      PeterKietzmann <peter.kietzmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>

#if !defined(COSY_TEST) && !defined(TEST_STACK)
#include <string.h>
#include "periph/gpio.h"
#include "xtimer.h"

gpio_t active_gpio = GPIO_PIN(1, 7);
gpio_t gpio_aes_key = GPIO_PIN(1, 8);
gpio_t gpio_sync_pin = GPIO_PIN(1, 6);

#define ITERATIONS                  (50)
#endif

#ifdef TEST_STACK
#include "ps.h"
#endif

#include "periph/hwrng.h"
#include "uECC.h"

#define CURVE_256_SIZE              (32)
#define PUB_KEY_SIZE                (CURVE_256_SIZE * 2)


struct uECC_Curve_t *curve;
uint8_t userPrivKey1[CURVE_256_SIZE];
uint8_t userPubKey1[PUB_KEY_SIZE];
uint8_t sharedSecret_01[CURVE_256_SIZE];

#if !defined(COSY_TEST) && !defined(TEST_STACK)
uint8_t userPrivKey2[CURVE_256_SIZE];
uint8_t userPubKey2[PUB_KEY_SIZE];
uint8_t sharedSecret_02[CURVE_256_SIZE];
#endif

static inline void _init_trigger(void)
{
#if TEST_ENERGY
    gpio_init(active_gpio, GPIO_OUT);
    gpio_init(gpio_aes_key, GPIO_OUT);
    gpio_init(gpio_sync_pin, GPIO_IN);

    gpio_set(active_gpio);
    gpio_clear(gpio_aes_key);
#else
    gpio_init(active_gpio, GPIO_OUT);
    gpio_clear(active_gpio);
#endif
}

static inline void _start_trigger(void)
{
#if TEST_ENERGY
    while(gpio_read(gpio_sync_pin)) {};
    while(!gpio_read(gpio_sync_pin)) {};
    gpio_clear(active_gpio);
#else
    gpio_set(active_gpio);
#endif
}

static inline void _stop_trigger(void)
{
#if TEST_ENERGY
    gpio_set(gpio_aes_key);

    gpio_set(active_gpio);
    gpio_clear(gpio_aes_key);
#else
    gpio_clear(active_gpio);
#endif
}

void _init_curve(void)
{
#if !defined(COSY_TEST) && !defined(TEST_STACK)
    _start_trigger();
    curve = (struct uECC_Curve_t*)uECC_secp256r1();
    _stop_trigger();
#else
    curve = (struct uECC_Curve_t*)uECC_secp256r1();
#endif
}

void _gen_keypair(void)
{
#if !defined(COSY_TEST) && !defined(TEST_STACK)
    int ret;
    _start_trigger();
    ret = uECC_make_key(userPubKey1, userPrivKey1, curve);
    _stop_trigger();
    if(!ret) {
        puts("ERROR generating Key 1");
        return;
    }
    ret = uECC_make_key(userPubKey2, userPrivKey2, curve);
    if(!ret) {
        puts("ERROR generating Key 2");
        return;
    }
#else
    uECC_make_key(userPubKey1, userPrivKey1, curve);
#endif
}

void _derive_shared_secret(void)
{
#if !defined(COSY_TEST) && !defined(TEST_STACK)
    int ret;

    _start_trigger();
    ret = uECC_shared_secret(userPubKey1, userPrivKey2, sharedSecret_01, curve);
    _stop_trigger();
    if(!ret) {
        puts("ERROR generating shared secret 1");
        return;
    }

    ret = uECC_shared_secret(userPubKey2, userPrivKey1, sharedSecret_02, curve);
    if(!ret) {
        puts("ERROR generating shared secret 2");
        return;
    }
    // generated secret should be the same on both
    if (memcmp(sharedSecret_01, sharedSecret_02, CURVE_256_SIZE)) {
        puts("ERROR");
    }
    else {
        puts("SUCCESS");
    }
#else
    uECC_shared_secret(userPubKey1, userPrivKey1, sharedSecret_01, curve);
#endif
}

int main(void)
{
#if !defined(COSY_TEST) && !defined(TEST_STACK)
    puts("'crypto-ewsn2020_ecdh uECC'");

    _init_trigger();

    // xtimer_sleep(2);

    for (int i = 0; i < ITERATIONS; i++) {
#endif
        _init_curve();
        // generate two instances of keypairs
        _gen_keypair();

        // derive and compare secrets generated on both
        _derive_shared_secret();
#if !defined(COSY_TEST) && !defined(TEST_STACK)
    }
#endif

#if defined(TEST_STACK)
    ps();
    printf("sizeof(userPrivKey1): %i\n", sizeof(userPrivKey1));
    printf("sizeof(userPubKey1): %i\n", sizeof(userPubKey1));
#endif
    puts("DONE");
    return 0;
}
