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

#if !defined(TEST_MEM) && !defined(TEST_STACK)
#include "periph/gpio.h"
#include "xtimer.h"

gpio_t active_gpio = GPIO_PIN(1, 7);
gpio_t gpio_aes_key = GPIO_PIN(1, 8);
gpio_t gpio_sync_pin = GPIO_PIN(1, 6);

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

#include "relic.h"

#if !defined(TEST_MEM) && !defined(TEST_STACK)
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
#if !defined(TEST_MEM) && !defined(TEST_STACK)
    // generate pubkey pair A
    _start_trigger();
    int ret = cp_ecdsa_gen(keyA.priv, keyA.pub);
    _stop_trigger();
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

#if !defined(TEST_MEM) && !defined(TEST_STACK)
    int ret;
    _start_trigger();
    md_map_sh256(hash, msg, ECDSA_MESSAGE_SIZE);
    _stop_trigger();


    // "0" selects message format without hashing. "1" would indicatehashing is needed
    _start_trigger();
    cp_ecdsa_sig(r, s, hash, sizeof(hash), 1, keyA.priv);
    _stop_trigger();


    // verify
    _start_trigger();
    ret = cp_ecdsa_ver(r, s, hash, sizeof(hash), 1, keyA.pub);
    _stop_trigger();

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
#ifndef TEST_STACK
extern ctx_t first_ctx;
// size of the precomputation table
printf("sizeof(first_ctx.ep_pre):%i\n", sizeof(first_ctx.ep_pre));
printf("EP_SIM: %d\n",EP_SIM);

#ifdef EP_PLAIN
    puts("EP_PLAIN on");
#endif
#ifdef EP_SUPER
    puts("EP_SUPER on");
#endif
#ifdef EP_ENDOM
    puts("EP_ENDOM on");
#endif
#ifdef EP_MIXED
    puts("EP_MIXED on");
#endif
#ifdef EP_PRECO
    puts("EP_PRECO on");
#else
    puts("EP_PRECO off");
#endif
printf("EP_DEPTH: %d\n",EP_DEPTH);
printf("EP_WIDTH: %d\n",EP_WIDTH);
printf("EP_METHD: %s\n",EP_METHD);
printf("RELIC_EP_TABLE: %i\n", RELIC_EP_TABLE);
#endif

#if !defined(TEST_MEM) && !defined(TEST_STACK)
    puts("'crypto-ewsn2020_ecdsa'");
    _init_trigger();

    // xtimer_sleep(1);

    for (int i = 0; i < ITERATIONS; i++){
        _start_trigger();
        _init_mem(&keyA);
        _stop_trigger();
#else
        _init_mem(&keyA);
#endif
        // generate keypairs
        _gen_keypair();
        // sign data and verify with public ley
        _sign_verify();

#if !defined(TEST_MEM) && !defined(TEST_STACK)
    }
#endif

#ifdef TEST_STACK
    ps();
    printf("sizeof(keyA.pub): %i\n", sizeof(keyA.pub));
    printf("sizeof(keyA.priv): %i\n", sizeof(keyA.priv));
#endif

    core_clean();
    puts("DONE");
    return 0;
}
