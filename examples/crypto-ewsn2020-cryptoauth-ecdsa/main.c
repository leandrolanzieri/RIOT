
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

#ifndef COSY_TEST
#include "periph/gpio.h"
#include "xtimer.h"
#include "ps.h"

gpio_t active_gpio = GPIO_PIN(1, 7);

#define ITERATIONS                  (50)
#endif

#define ECDSA_MESSAGE_SIZE          (127)

uint8_t UserPubKey[ATCA_PUB_KEY_SIZE];
uint8_t key_id = 1; /* This is the number of the slot used */

void _gen_keypair(void)
{
#ifndef COSY_TEST
    ATCA_STATUS status;
    gpio_set(active_gpio);
    status = atcab_genkey(key_id, UserPubKey);
    gpio_clear(active_gpio);
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

#ifndef COSY_TEST
    ATCA_STATUS status;
    gpio_set(active_gpio);
    atcab_hw_sha2_256(msg, ECDSA_MESSAGE_SIZE, hash);
    gpio_clear(active_gpio);

    gpio_set(active_gpio);
    status = atcab_sign(key_id, msg, signature);
    gpio_clear(active_gpio);
    if (status != ATCA_SUCCESS){
        printf(" Signing failed with 0x%x \n",status);
        return;
    }

    gpio_set(active_gpio);
    status = atcab_verify_extern(msg, signature, UserPubKey, &is_verified);
    gpio_clear(active_gpio);
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
#ifndef COSY_TEST
    puts("'crypto-ewsn2020_ecdsa'");

    gpio_init(active_gpio, GPIO_OUT);
    gpio_clear(active_gpio);

    xtimer_sleep(1);

    for (int i = 0; i < ITERATIONS; i++) {
        gpio_set(active_gpio);
        // Empty gpio set instead of init
        gpio_clear(active_gpio);
#endif
        // generate two instances of keypairs
        _gen_keypair();

        // derive and compare secrets generated on both
        _sign_verify();
#ifndef COSY_TEST
    }

    ps();
    printf("sizeof(UserPubKey): %i\n", sizeof(UserPubKey));
#endif
    puts("DONE");
    return 0;
}