
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
#include <assert.h>
#include <stdlib.h>

#include "atca.h"
#include "atca_util.h"
#include "atca_params.h"
#include "atca_command.h"
#include "host/atca_host.h"
#include "basic/atca_basic.h"
#include "atca_execution.h"
#include "periph/gpio.h"

ATCA_STATUS status;
uint8_t SharedSecret1[ECDH_KEY_SIZE];
uint8_t SharedSecret2[ECDH_KEY_SIZE];
uint8_t UserPubKey1[ATCA_PUB_KEY_SIZE];
uint8_t UserPubKey2[ATCA_PUB_KEY_SIZE];
uint8_t read_key[ATCA_KEY_SIZE];
uint8_t key_id_1 = 2;
uint8_t key_id_2 = 1;

void _gen_keypair(void)
{
#ifdef ATCA_MANUAL_ONOFF
    atecc_wake();
    status = atcab_genkey(key_id_1, UserPubKey1);
    atecc_idle();
#else
    status = atcab_genkey(key_id_1, UserPubKey1);
#endif
    if (status != ATCA_SUCCESS){
        printf(" atcab_genkey for PubKey1 failed with 0x%x \n",status);
        return;
    }

#ifdef ATCA_MANUAL_ONOFF
    atecc_wake();
    status = atcab_genkey(key_id_2, UserPubKey2);
    atecc_idle();
#else
    status = atcab_genkey(key_id_2, UserPubKey2);
#endif
    if (status != ATCA_SUCCESS){
        printf(" atcab_genkey for PubKey2 failed with 0x%x \n",status);
        return;
    }
}

void _derive_shared_secret(void)
{
#ifdef ATCA_MANUAL_ONOFF
    atecc_wake();
    status = atcab_ecdh(key_id_1, UserPubKey2, SharedSecret1);
    atecc_idle();
#else
    status = atcab_ecdh(key_id_1, UserPubKey2, SharedSecret1);
#endif
    if (status != ATCA_SUCCESS){
        printf(" atcab_ecdh for secret 2 failed with 0x%x \n",status);
        return;
    }

#ifdef ATCA_MANUAL_ONOFF
    atecc_wake();
    status = atcab_ecdh(key_id_2, UserPubKey1, SharedSecret2);
    atecc_sleep();
#else
    status = atcab_ecdh(key_id_2, UserPubKey1, SharedSecret2);
#endif
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
}

int main(void)
{
    puts("'crypto-ewsn2020_ecdh'");

    // generate two instances of keypairs
    _gen_keypair();

    // derive and compare secrets generated on both
    _derive_shared_secret();
    return 0;
}