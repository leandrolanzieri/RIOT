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

#include "relic.h"

typedef struct
{
    ec_t pub;
    bn_t priv;
} key_struct_t;

static key_struct_t keyA;

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
    // generate pubkey pair A
    int ret = cp_ecdsa_gen(keyA.priv, keyA.pub);
    if (ret == STS_ERR) {
        printf("ECDH key pair creation failed. Not good :( \n");
        return;
    }
}

void _sign_verify(void)
{
    // this is a test message copied from relic tests
    uint8_t m[5] = { 0, 1, 2, 3, 4 };

    bn_t r, s;
    bn_new(r);
    bn_new(s);

    // "0" selects message format without hashing. "1" would indicatehashing is needed
    cp_ecdsa_sig(r, s, m, sizeof(m), 0, keyA.priv);

    // verify
    if (cp_ecdsa_ver(r, s, m, sizeof(m), 0, keyA.pub) == 1)
    {
        puts("VALID");
    }
    else
    {
        puts("INVALID");
    }
}

int main(void)
{
    puts("'crypto-ewsn2020_ecdsa'");
    core_init();

    // set NIST P-256 elliptic curve
    ep_param_set(NIST_P256);
    ec_param_print();

    // init memory
    _init_mem(&keyA);

    // generate keypairs
    _gen_keypair();

    // sign data and verify with public ley
    _sign_verify();

    core_clean();
    return 0;
}
