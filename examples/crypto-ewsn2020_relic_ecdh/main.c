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

#if !defined(TEST_MEM) && !defined(TEST_STACK)
#include "xtimer.h"
#include "periph/gpio.h"

gpio_t active_gpio = GPIO_PIN(1, 7);
gpio_t gpio_aes_key = GPIO_PIN(1, 8);
gpio_t gpio_sync_pin = GPIO_PIN(1, 6);
#endif

#ifdef TEST_STACK
#include "ps.h"
#endif

#include "relic.h"

#if !defined(TEST_MEM) && !defined(TEST_STACK)
#define ITERATIONS                  (50)
#endif

typedef struct
{
    ec_t pub;
    bn_t priv;
} key_struct_t;

static key_struct_t keyA;

#if !defined(TEST_MEM) && !defined(TEST_STACK)
static key_struct_t keyB;

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
void _init_mem(key_struct_t *key)
{
    // set up memory
    bn_null(key->priv);
    ec_null(key->pub);

    bn_new(key->priv);
    ec_new(key->pub);
}

void _gen_keypair(void)
{
#if !defined(TEST_MEM) && !defined(TEST_STACK)
    // generate pubkey pair A
    _start_trigger();
    int ret = cp_ecdh_gen(keyA.priv, keyA.pub);
    _stop_trigger();
    if (ret == STS_ERR) {
        puts("ECDH key pair creation failed. Not good :( \n");
        return;
    }

    // generate pubkey pair B
    ret = cp_ecdh_gen(keyB.priv, keyB.pub);
    if (ret == STS_ERR) {
        puts("ECDH key pair creation failed. Not good :( \n");
        return;
    }
#else
    cp_ecdh_gen(keyA.priv, keyA.pub);
#endif
}

void _derive_shared_secret(void)
{
    uint8_t sharedKeyA[MD_LEN];

#if !defined(TEST_MEM) && !defined(TEST_STACK)
    uint8_t sharedKeyB[MD_LEN];

    // generate shared secred on A, based on priv key B
    _start_trigger();
    int ret = cp_ecdh_key(sharedKeyA, MD_LEN, keyA.priv, keyB.pub);
    _stop_trigger();
    if (ret == STS_ERR) {
        puts("ECDH secret A creation failed. Not good :( \n");
        return;
    }

    // generate shared secred on B, based on priv key A
    ret = cp_ecdh_key(sharedKeyB, MD_LEN, keyB.priv, keyA.pub);
    if (ret == STS_ERR) {
        puts("ECDH secret B creation failed. Not good :( \n");
        return;
    }

    // generated secret should be the same on both
    if (memcmp(sharedKeyA, sharedKeyB, MD_LEN)) {
        puts("ERROR");
    }
    else {
        puts("SUCCESS");
    }
#else
    cp_ecdh_key(sharedKeyA, MD_LEN, keyA.priv, keyA.pub);
#endif
}

int main(void)
{
    core_init();

#if !defined(TEST_MEM) && !defined(TEST_STACK)
    puts("'crypto-ewsn2020_ecdh'");

    _init_trigger();

    // xtimer_sleep(1);

    for (int i = 0; i < ITERATIONS; i++) {
        // Init Curve and KeyA
        _start_trigger();
        ep_param_set(NIST_P256);
        _init_mem(&keyA);
       _stop_trigger();

        // Init KeyB
        _init_mem(&keyB);
#else
        ep_param_set(NIST_P256);
        _init_mem(&keyA);
#endif
        // generate two instances of keypairs
        _gen_keypair();

        // derive and compare secrets generated on both
        _derive_shared_secret();

#if !defined(TEST_MEM) && !defined(TEST_STACK)
    }
#endif

#if defined(TEST_STACK)
    ps();
    printf("sizeof(keyA): %i\n", sizeof(keyA));
#endif

    core_clean();
    puts("DONE");
    return 0;
}
