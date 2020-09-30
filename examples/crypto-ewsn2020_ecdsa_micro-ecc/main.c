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

#include "hashes/sha256.h"
#include "periph/hwrng.h"
#include "uECC.h"

#ifndef COSY_TEST
#include "periph/gpio.h"
#include "xtimer.h"
#include "ps.h"

gpio_t active_gpio = GPIO_PIN(1, 7);
#define ITERATIONS                  (50)
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
#ifndef COSY_TEST
    gpio_set(active_gpio);
    curve = (struct uECC_Curve_t*)uECC_secp256r1();
    gpio_clear(active_gpio);
#else
    curve = (struct uECC_Curve_t*)uECC_secp256r1();
#endif
}

void _gen_keypair(void)
{
#ifndef COSY_TEST
    int ret;
    gpio_set(active_gpio);
    ret = uECC_make_key(userPubKey1, userPrivKey1, curve);
    gpio_clear(active_gpio);
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

#ifndef COSY_TEST
    int ret;
    gpio_set(active_gpio);
    sha256(msg, ECDSA_MESSAGE_SIZE, hash);
    gpio_clear(active_gpio);

    gpio_set(active_gpio);
    ret = uECC_sign(userPrivKey1, hash, SHA256_DIGEST_SIZE, signature, curve);
    gpio_clear(active_gpio);
    if(ret != 1) {
        puts("ERROR generating shared secret 1");
        return;
    }
    gpio_set(active_gpio);
    ret = uECC_verify(userPubKey1, hash, SHA256_DIGEST_SIZE, signature, curve);
    gpio_clear(active_gpio);
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
#ifndef COSY_TEST
    puts("'crypto-ewsn2020_ecdh uECC'");

    gpio_init(active_gpio, GPIO_OUT);
    gpio_clear(active_gpio);

    xtimer_sleep(1);

    for (int i = 0; i < ITERATIONS; i++) {
#endif
        _init_curve();
        // generate two instances of keypairs
        _gen_keypair();

        // derive and compare secrets generated on both
        _sign_verify();
#ifndef COSY_TEST
    }

    ps();
    printf("sizeof(userPrivKey1): %i\n", sizeof(userPrivKey1));
    printf("sizeof(userPubKey1): %i\n", sizeof(userPubKey1));
#endif
    puts("DONE");
    return 0;
}
