
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

#include "atca.h"
#include "atca_params.h"
#include "atca_command.h"
#include "host/atca_host.h"
#include "basic/atca_basic.h"
#include "atca_execution.h"

#ifdef TEST_STACK
#include "ps.h"
#endif

#if !defined(COSY_TEST) && !defined(TEST_STACK)
#include "xtimer.h"
#include "periph/gpio.h"

gpio_t active_gpio = GPIO_PIN(1, 7);

#define ITERATIONS                  (50)
#endif

uint8_t UserPubKey1[ATCA_PUB_KEY_SIZE];
uint8_t key_id_1 = 2;

#if !defined(COSY_TEST) && !defined(TEST_STACK)
uint8_t UserPubKey2[ATCA_PUB_KEY_SIZE];
uint8_t key_id_2 = 1;
#endif

void _gen_keypair(void)
{
#if !defined(COSY_TEST) && !defined(TEST_STACK)
    ATCA_STATUS status;
    gpio_set(active_gpio);
    status = atcab_genkey(key_id_1, UserPubKey1);
    gpio_clear(active_gpio);
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

#if !defined(COSY_TEST) && !defined(TEST_STACK)
    uint8_t SharedSecret2[ECDH_KEY_SIZE];

    ATCA_STATUS status;
    gpio_set(active_gpio);
    status = atcab_ecdh(key_id_1, UserPubKey2, SharedSecret1);
    gpio_clear(active_gpio);
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
#if !defined(COSY_TEST) && !defined(TEST_STACK)
    puts("'crypto-ewsn2020_cryptocell_ecdh'");
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
        _derive_shared_secret();
#if !defined(COSY_TEST) && !defined(TEST_STACK)
    }
#endif
#if defined(TEST_STACK)
    ps();
    printf("sizeof(UserPubKey1): %i\n", sizeof(UserPubKey1));
#endif
    puts("DONE");
    return 0;
}