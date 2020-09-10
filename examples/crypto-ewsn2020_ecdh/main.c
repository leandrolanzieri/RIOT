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
static key_struct_t keyB;

void print_mem(void *mem, int len) {
  int i;
  unsigned char *p = (unsigned char *)mem;
  for (i=0;i<len;i++) {
    printf("0x%02x ", p[i]);
  }
  printf("\n");
}

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
    int ret = cp_ecdh_gen(keyA.priv, keyA.pub);
    if (ret == STS_ERR) {
        printf("ECDH key pair creation failed. Not good :( \n");
        return;
    }

    // generate pubkey pair B
    ret = cp_ecdh_gen(keyB.priv, keyB.pub);
    if (ret == STS_ERR) {
        printf("ECDH key pair creation failed. Not good :( \n");
        return;
    }
}

void _derive_shared_secret(void)
{
    printf("Length of shared secret: %i\n", MD_LEN);

    uint8_t sharedKeyA[MD_LEN];
    uint8_t sharedKeyB[MD_LEN];

    // generate shared secred on A, based on priv key B
    int ret = cp_ecdh_key(sharedKeyA, MD_LEN, keyA.priv, keyB.pub);
    if (ret == STS_ERR) {
        printf("ECDH secret A creation failed. Not good :( \n");
        return;
    }

    // generate shared secred on B, based on priv key A
    ret = cp_ecdh_key(sharedKeyB, MD_LEN, keyB.priv, keyA.pub);
        if (ret == STS_ERR) {
        printf("ECDH secret B creation failed. Not good :( \n");
        return;
    }

    // generated secret should be the same on both
    if (memcmp(sharedKeyA, sharedKeyB, MD_LEN)) {
        puts("ERROR");
    }
    else {
        puts("SUCCESS");
    }
}

int main(void)
{
    puts("'crypto-ewsn2020_ecdh'");
    core_init();

    // set NIST P-256 elliptic curve
    ep_param_set(NIST_P256);
    ec_param_print();

    // init memory
    _init_mem(&keyA);
    _init_mem(&keyB);

    // generate two instances of keypairs
    _gen_keypair();

    // derive and compare secrets generated on both
    _derive_shared_secret();

    core_clean();
    return 0;
}
