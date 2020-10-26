
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

#include "atca_command.h"
#include "host/atca_host.h"
#include "basic/atca_basic.h"
#include "atca_execution.h"

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

uint8_t UserPubKey[ATCA_PUB_KEY_SIZE];
uint8_t key_id = 1; /* This is the number of the slot used */



void _gen_keypair(void)
{
#if !defined(TEST_MEM) && !defined(TEST_STACK)
    ATCA_STATUS status;
    _start_trigger();
    status = atcab_genkey(key_id, UserPubKey);
    _stop_trigger();
    if (status != ATCA_SUCCESS){
        printf(" atcab_genkey for PubKey1 failed with 0x%x \n",status);
        return;
    }
#else
    atcab_genkey(key_id, UserPubKey);
#endif
}

void _sign_verify(void)
{
    uint8_t signature[ATCA_SIG_SIZE];
    bool is_verified = false;

    /* atcab_sign expects the message to be pre hashed and only computes input with the size of a SHA256 digest correctly */
    uint8_t msg[ECDSA_MESSAGE_SIZE] = { 0x0b };
    uint8_t hash[ATCA_SHA_DIGEST_SIZE];

#if !defined(TEST_MEM) && !defined(TEST_STACK)
    ATCA_STATUS status;
    _start_trigger();
    atcab_hw_sha2_256(msg, ECDSA_MESSAGE_SIZE, hash);
    _stop_trigger();

    _start_trigger();
    status = atcab_sign(key_id, msg, signature);
    _stop_trigger();
    if (status != ATCA_SUCCESS){
        printf(" Signing failed with 0x%x \n",status);
        return;
    }

    _start_trigger();
    status = atcab_verify_extern(msg, signature, UserPubKey, &is_verified);
    _stop_trigger();
    if (status != ATCA_SUCCESS){
        printf(" Verifying failed with 0x%x \n",status);
        return;
    }
    if (is_verified) {
        puts("VALID");
    }
    else {
        puts("INVALID");
    }
#else
    atcab_hw_sha2_256(msg, ECDSA_MESSAGE_SIZE, hash);
    atcab_sign(key_id, msg, signature);
    atcab_verify_extern(msg, signature, UserPubKey, &is_verified);
#endif
}

int main(void)
{
#if !defined(TEST_MEM) && !defined(TEST_STACK)
    puts("'crypto-ewsn2020_ecdsa'");

    _init_trigger();

    for (int i = 0; i < ITERATIONS; i++) {
        _start_trigger();
        // Empty gpio set instead of init
        _stop_trigger();
#endif
        // generate two instances of keypairs
        _gen_keypair();

        // derive and compare secrets generated on both
        _sign_verify();
#if !defined(TEST_MEM) && !defined(TEST_STACK)
    }
#endif

#ifdef TEST_STACK
    ps();
    printf("sizeof(UserPubKey): %i\n", sizeof(UserPubKey));
#endif
    puts("DONE");
    return 0;
}