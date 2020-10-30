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
 * @brief       this is an ecdsa test application
 *
 * @author      Lena Boeckmann <lena.boeckmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>

#include "hashes/sha256.h"
#include "periph/hwrng.h"
#include "uECC.h"

#ifdef TEST_STACK
#include "ps.h"
#endif

#if !defined(TEST_MEM) && !defined(TEST_STACK)
#include "periph/gpio.h"
#include "xtimer.h"

gpio_t active_gpio = GPIO_PIN(1, 7);
gpio_t gpio_aes_key = GPIO_PIN(1, 8);
gpio_t gpio_sync_pin = GPIO_PIN(1, 6);
#define ITERATIONS                  (50)


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

#endif

#define ECDSA_MESSAGE_SIZE          (127)
#define SHA256_DIGEST_SIZE          (32)
#define CURVE_256_SIZE              (32)
#define PUB_KEY_SIZE                (CURVE_256_SIZE * 2)

struct uECC_Curve_t *curve;
uint8_t userPrivKey1[CURVE_256_SIZE];
uint8_t userPubKey1[PUB_KEY_SIZE];



void _init_curve(void)
{
#if !defined(TEST_MEM) && !defined(TEST_STACK)
    _start_trigger();
    curve = (struct uECC_Curve_t*)uECC_secp256r1();
    _stop_trigger();
#else
    curve = (struct uECC_Curve_t*)uECC_secp256r1();
#endif
}

void _gen_keypair(void)
{
#if !defined(TEST_MEM) && !defined(TEST_STACK)
    int ret;
    _start_trigger();
    ret = uECC_make_key(userPubKey1, userPrivKey1, curve);
    _stop_trigger();
    if(!ret) {
        puts("ERROR generating Key 1");
        return;
    }
#else
    uECC_make_key(userPubKey1, userPrivKey1, curve);
#endif
}

void _sign_verify(void)
{
    uint8_t msg[ECDSA_MESSAGE_SIZE] = { 0x0b };
    uint8_t hash[SHA256_DIGEST_SIZE];
    uint8_t signature[PUB_KEY_SIZE];

#if !defined(TEST_MEM) && !defined(TEST_STACK)
    int ret;
    _start_trigger();
    sha256(msg, ECDSA_MESSAGE_SIZE, hash);
    _stop_trigger();

    _start_trigger();
    ret = uECC_sign(userPrivKey1, hash, SHA256_DIGEST_SIZE, signature, curve);
    _stop_trigger();
    if(ret != 1) {
        puts("ERROR generating shared secret 1");
        return;
    }
    _start_trigger();
    ret = uECC_verify(userPubKey1, hash, SHA256_DIGEST_SIZE, signature, curve);
    _stop_trigger();
    if(ret != 1) {
        puts("INVALID");
    }
    else
    {
        puts("VALID");
    }
#else
    sha256(msg, ECDSA_MESSAGE_SIZE, hash);
    uECC_sign(userPrivKey1, hash, SHA256_DIGEST_SIZE, signature, curve);
    uECC_verify(userPubKey1, hash, SHA256_DIGEST_SIZE, signature, curve);
#endif
}

int main(void)
{
#if !defined(TEST_MEM) && !defined(TEST_STACK)
    puts("'crypto-ewsn2020_ecdsa uECC'");

    _init_trigger();

    // xtimer_sleep(1);

    for (int i = 0; i < ITERATIONS; i++) {
#endif
        _init_curve();
        // generate two instances of keypairs
        _gen_keypair();

        // derive and compare secrets generated on both
        _sign_verify();
#if !defined(TEST_MEM) && !defined(TEST_STACK)
    }
#endif
#ifdef TEST_STACK
    ps();
    printf("sizeof(userPrivKey1): %i\n", sizeof(userPrivKey1));
    printf("sizeof(userPubKey1): %i\n", sizeof(userPubKey1));
#endif
    puts("DONE");
    return 0;
}
