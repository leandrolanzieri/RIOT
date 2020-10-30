
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
 * @author      Lena Boeckmann <lena.boeckmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>

#include "atca.h"
#include "atca_params.h"
#include "atca_command.h"
#include "host/atca_host.h"
#include "basic/atca_basic.h"
#include "atca_execution.h"

#ifdef TEST_STACK
#include "ps.h"
#endif

#if !defined(TEST_MEM) && !defined(TEST_STACK)
#include "xtimer.h"
#include "periph/gpio.h"

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

uint8_t UserPubKey2[ATCA_PUB_KEY_SIZE];
uint8_t key_id_2 = 1;

#endif

uint8_t UserPubKey1[ATCA_PUB_KEY_SIZE];
uint8_t key_id_1 = 2;

void _gen_keypair(void)
{
#if !defined(TEST_MEM) && !defined(TEST_STACK)
    ATCA_STATUS status;
    _start_trigger();
    status = atcab_genkey(key_id_1, UserPubKey1);
     _stop_trigger();
    if (status != ATCA_SUCCESS){
        printf(" atcab_genkey for PubKey1 failed with 0x%x \n",status);
        return;
    }
    status = atcab_genkey(key_id_2, UserPubKey2);
    if (status != ATCA_SUCCESS){
        printf(" atcab_genkey for PubKey2 failed with 0x%x \n",status);
        return;
    }
#else
    atcab_genkey(key_id_1, UserPubKey1);
#endif
}

void _derive_shared_secret(void)
{
    uint8_t SharedSecret1[ECDH_KEY_SIZE];

#if !defined(TEST_MEM) && !defined(TEST_STACK)
    uint8_t SharedSecret2[ECDH_KEY_SIZE];

    ATCA_STATUS status;
    _start_trigger();
    status = atcab_ecdh(key_id_1, UserPubKey2, SharedSecret1);
     _stop_trigger();
    if (status != ATCA_SUCCESS){
        printf(" atcab_ecdh for secret 2 failed with 0x%x \n",status);
        return;
    }
    status = atcab_ecdh(key_id_2, UserPubKey1, SharedSecret2);
    if (status != ATCA_SUCCESS){
        printf(" atcab_ecdh for secret 2 failed with 0x%x \n",status);
        return;
    }
    // generated secret should be the same on both
    if (memcmp(SharedSecret1, SharedSecret2, sizeof(SharedSecret1))) {
        puts("ERROR");
    }
    else {
        puts("SUCCESS");
    }
#else
    atcab_ecdh(key_id_1, UserPubKey1, SharedSecret1);
#endif
}

int main(void)
{
#if !defined(TEST_MEM) && !defined(TEST_STACK)
    puts("'crypto-ewsn2020_cryptocell_ecdh'");

    _init_trigger();

    for (int i = 0; i < ITERATIONS; i++) {

        _start_trigger();
        // Empty gpio set instead of init
         _stop_trigger();
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
    printf("sizeof(UserPubKey1): %i\n", sizeof(UserPubKey1));
#endif
    puts("DONE");
    return 0;
}