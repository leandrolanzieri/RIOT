
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

#include "atca_util.h"
#include "atca_command.h"
#include "host/atca_host.h"
#include "basic/atca_basic.h"
#include "atca_execution.h"

ATCA_STATUS status;
uint8_t UserPubKey[ATCA_PUB_KEY_SIZE];
uint8_t key_id = 1; /* This is the number of the slot used */

void _gen_keypair(void)
{
#ifdef ATCA_MANUAL_ONOFF
    atecc_wake();
    status = atcab_genkey(key_id, UserPubKey);
    atecc_sleep();
#else
    status = atcab_genkey(key_id, UserPubKey);
#endif
    if (status != ATCA_SUCCESS){
        printf(" atcab_genkey for PubKey1 failed with 0x%x \n",status);
        return;
    }
}

void _sign_verify(void)
{
    uint8_t signature[ATCA_SIG_SIZE];
    bool is_verified = false;

    /* atcab_sign expects the message to be pre hashed and only computes input with the size of a SHA256 digest correctly */
    uint8_t msg[ATCA_SHA_DIGEST_SIZE] = { 0x0b };

#ifdef ATCA_MANUAL_ONOFF
    atecc_wake();
    status = atcab_sign(key_id, msg, signature);
    atecc_sleep();
#else
    status = atcab_sign(key_id, msg, signature);
#endif
    if (status != ATCA_SUCCESS){
        printf(" Signing failed with 0x%x \n",status);
        return;
    }

#ifdef ATCA_MANUAL_ONOFF
    atecc_wake();
    status = atcab_verify_extern(msg, signature, UserPubKey, &is_verified);
    atecc_sleep();
#else
    status = atcab_verify_extern(msg, signature, UserPubKey, &is_verified);
#endif
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
}

int main(void)
{
    puts("'crypto-ewsn2020_ecdsa'");

    // generate two instances of keypairs
    _gen_keypair();

    // derive and compare secrets generated on both
    _sign_verify();
    return 0;
}