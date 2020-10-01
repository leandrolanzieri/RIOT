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

#ifdef TEST_STACK
#include "ps.h"
#endif

#if !defined(COSY_TEST) && !defined(TEST_STACK)
#include "periph/gpio.h"
#include "xtimer.h"

gpio_t active_gpio = GPIO_PIN(1, 7);
#endif

#include "relic.h"

#if !defined(COSY_TEST) && !defined(TEST_STACK)
#define ITERATIONS          (50)
#endif

#define SHA256_DIGEST_SIZE  (32)
#define ECDSA_MESSAGE_SIZE  (127)

typedef struct
{
    ec_t pub;
    bn_t priv;
} key_struct_t;

static key_struct_t keyA;

void _init_mem(key_struct_t *key)
{
    ep_param_set(NIST_P256);
    // set up memory
    bn_null(key->priv);
    ec_null(key->pub);

    bn_new(key->priv);
    ec_new(key->pub);
}

void _gen_keypair(void)
{
#if !defined(COSY_TEST) && !defined(TEST_STACK)
    // generate pubkey pair A
    gpio_set(active_gpio);
    int ret = cp_ecdsa_gen(keyA.priv, keyA.pub);
    gpio_clear(active_gpio);
    if (ret == STS_ERR) {
        puts("ECDH key pair creation failed. Not good :( \n");
        return;
    }
#else
    cp_ecdsa_gen(keyA.priv, keyA.pub);
#endif
}

void _sign_verify(void)
{
    uint8_t msg[ECDSA_MESSAGE_SIZE] = { 0x0b };
    uint8_t hash[SHA256_DIGEST_SIZE];

    bn_t r, s;
    bn_new(r);
    bn_new(s);

#if !defined(COSY_TEST) && !defined(TEST_STACK)
    int ret;
    gpio_set(active_gpio);
    md_map_sh256(hash, msg, ECDSA_MESSAGE_SIZE);
    gpio_clear(active_gpio);

    // "0" selects message format without hashing. "1" would indicatehashing is needed
    gpio_set(active_gpio);
    cp_ecdsa_sig(r, s, hash, sizeof(hash), 0, keyA.priv);
    gpio_clear(active_gpio);

    // verify
    gpio_set(active_gpio);
    ret = cp_ecdsa_ver(r, s, hash, sizeof(hash), 0, keyA.pub);
    gpio_clear(active_gpio);

    if (ret == 1) {
        puts("VALID");
    }
    else {
        puts("INVALID");
    }
#else
    md_map_sh256(hash, msg, ECDSA_MESSAGE_SIZE);
    cp_ecdsa_sig(r, s, hash, sizeof(hash), 0, keyA.priv);
    cp_ecdsa_ver(r, s, hash, sizeof(hash), 0, keyA.pub);
#endif
}

int main(void)
{
    core_init();

#if !defined(COSY_TEST) && !defined(TEST_STACK)
    puts("'crypto-ewsn2020_ecdsa'");
    gpio_init(active_gpio, GPIO_OUT);
    gpio_clear(active_gpio);

    xtimer_sleep(1);

    for (int i = 0; i < ITERATIONS; i++){

        gpio_set(active_gpio);
        _init_mem(&keyA);
        gpio_clear(active_gpio);
#else
        _init_mem(&keyA);
#endif
        // generate keypairs
        _gen_keypair();

        // sign data and verify with public ley
        _sign_verify();

#if !defined(COSY_TEST) && !defined(TEST_STACK)
    }
#endif

#ifdef TEST_STACK
    ps();
    printf("sizeof(keyA): %i\n", sizeof(keyA));
#endif

    core_clean();
    puts("DONE");
    return 0;
}
