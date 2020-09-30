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

#ifndef COSY_TEST
#include "xtimer.h"
#include "ps.h"
#include "periph/gpio.h"

gpio_t active_gpio = GPIO_PIN(1, 7);
#endif

#include <stdio.h>
#include "relic.h"

#ifndef COSY_TEST
#define ITERATIONS                  (50)
#endif

typedef struct
{
    ec_t pub;
    bn_t priv;
} key_struct_t;

static key_struct_t keyA;

#ifndef COSY_TEST
static key_struct_t keyB;
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
#ifndef COSY_TEST
    // generate pubkey pair A
    gpio_set(active_gpio);
    int ret = cp_ecdh_gen(keyA.priv, keyA.pub);
    gpio_clear(active_gpio);
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

#ifndef COSY_TEST
    uint8_t sharedKeyB[MD_LEN];

    // generate shared secred on A, based on priv key B
    gpio_set(active_gpio);
    int ret = cp_ecdh_key(sharedKeyA, MD_LEN, keyA.priv, keyB.pub);
    gpio_clear(active_gpio);
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

#ifndef COSY_TEST
    puts("'crypto-ewsn2020_ecdh'");

    gpio_init(active_gpio, GPIO_OUT);
    gpio_clear(active_gpio);

    xtimer_sleep(1);

    for (int i = 0; i < ITERATIONS; i++) {
        // Init Curve and KeyA
        gpio_set(active_gpio);
        ep_param_set(NIST_P256);
        _init_mem(&keyA);
        gpio_clear(active_gpio);

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

#ifndef COSY_TEST
    }

    ps();
    printf("sizeof(keyA): %i\n", sizeof(keyA));
    printf("sizeof(keyB): %i\n", sizeof(keyB));
#endif

    core_clean();
    puts("DONE");
    return 0;
}
