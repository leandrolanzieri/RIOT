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
 * @brief       this is an ecdsa test application for cryptocell
 *
 * @author
 *
 * @}
 */

#include <stdio.h>
#include "cryptocell_util.h"

#include "cryptocell_incl/sns_silib.h"
#include "cryptocell_incl/crys_ecpki_build.h"
#include "cryptocell_incl/crys_ecpki_dh.h"
#include "cryptocell_incl/crys_ecpki_kg.h"
#include "cryptocell_incl/crys_ecpki_domain.h"

#ifdef TEST_STACK
#include "ps.h"
#endif

#if !defined(COSY_TEST) && !defined(TEST_STACK)
#include <string.h>
#include "periph/gpio.h"
#include "xtimer.h"

gpio_t active_gpio = GPIO_PIN(1, 7);

#define ITERATIONS                  (50)
#endif

#define SHARED_SECRET_LENGHT    (32)

extern CRYS_RND_State_t*     rndState_ptr;

CRYS_ECPKI_UserPrivKey_t UserPrivKey1;
CRYS_ECPKI_UserPublKey_t UserPublKey1;
CRYS_ECPKI_Domain_t* pDomain;
CRYS_ECDH_TempData_t* TempDHBuffptr;
CRYS_ECPKI_KG_TempData_t* TempECCKGBuffptr;
CRYS_ECDH_TempData_t TempDHBuff;
CRYS_ECPKI_KG_TempData_t TempECCKGBuff;
CRYS_ECPKI_KG_FipsContext_t FipsBuff;
SaSiRndGenerateVectWorkFunc_t rndGenerateVectFunc;
uint8_t sharedSecret1ptr[SHARED_SECRET_LENGHT];
uint32_t sharedSecret1Size = SHARED_SECRET_LENGHT;

#if !defined(COSY_TEST) && !defined(TEST_STACK)
CRYS_ECPKI_UserPrivKey_t UserPrivKey2;
CRYS_ECPKI_UserPublKey_t UserPublKey2;
uint8_t sharedSecret2ptr[SHARED_SECRET_LENGHT];
uint32_t sharedSecret2Size = SHARED_SECRET_LENGHT;
#endif

void _init_vars(void)
{
    rndGenerateVectFunc = CRYS_RND_GenerateVector;
    TempDHBuffptr = (CRYS_ECDH_TempData_t*)&TempDHBuff;
    TempECCKGBuffptr = (CRYS_ECPKI_KG_TempData_t*)&TempECCKGBuff;
    pDomain = (CRYS_ECPKI_Domain_t*)CRYS_ECPKI_GetEcDomain(CRYS_ECPKI_DomainID_secp256r1);
}

void _gen_keypair(void)
{
#if !defined(COSY_TEST) && !defined(TEST_STACK)
    int ret = 0;

    cryptocell_enable();
    gpio_set(active_gpio);
    ret = CRYS_ECPKI_GenKeyPair (rndState_ptr, rndGenerateVectFunc, pDomain, &UserPrivKey1, &UserPublKey1, TempECCKGBuffptr, &FipsBuff);
    gpio_clear(active_gpio);
    cryptocell_disable();
    if (ret != SA_SILIB_RET_OK){
        printf("CRYS_ECPKI_GenKeyPair for key pair 1 failed with 0x%x \n",ret);
        return;
    }
    cryptocell_enable();
    ret = CRYS_ECPKI_GenKeyPair (rndState_ptr, rndGenerateVectFunc, pDomain, &UserPrivKey2, &UserPublKey2, TempECCKGBuffptr, &FipsBuff);
    cryptocell_disable();
    if (ret != SA_SILIB_RET_OK){
        printf("CRYS_ECPKI_GenKeyPair for key pair 2 failed with 0x%x \n",ret);
        return;
    }
#else
    cryptocell_enable();
    CRYS_ECPKI_GenKeyPair (rndState_ptr, rndGenerateVectFunc, pDomain, &UserPrivKey1, &UserPublKey1, TempECCKGBuffptr, &FipsBuff);
    cryptocell_disable();
#endif
}

void _derive_shared_secret(void)
{
#if !defined(COSY_TEST) && !defined(TEST_STACK)
    int ret = 0;
    /* Generating the Secret for user 1*/
    /*---------------------------------*/
    cryptocell_enable();

    gpio_set(active_gpio);
    ret = CRYS_ECDH_SVDP_DH(&UserPublKey2, &UserPrivKey1, sharedSecret1ptr, &sharedSecret1Size, TempDHBuffptr);
    gpio_clear(active_gpio);

    cryptocell_disable();

    if (ret != SA_SILIB_RET_OK){
        printf(" CRYS_ECDH_SVDP_DH for secret 1 failed with 0x%x \n",ret);
        return;
    }

    /* Generating the Secret for user 2*/
    /*---------------------------------*/
    cryptocell_enable();
    ret = CRYS_ECDH_SVDP_DH(&UserPublKey1, &UserPrivKey2, sharedSecret2ptr, &sharedSecret2Size, TempDHBuffptr);
    cryptocell_disable();

    if (ret != SA_SILIB_RET_OK){
        printf(" CRYS_ECDH_SVDP_DH for secret 2 failed with 0x%x \n",ret);
        return;
    }
    // generated secret should be the same on both
    if (memcmp(sharedSecret1ptr, sharedSecret2ptr, SHARED_SECRET_LENGHT)) {
        puts("ERROR");
    }
    else {
        puts("SUCCESS");
    }
#else
    cryptocell_enable();
    CRYS_ECDH_SVDP_DH(&UserPublKey1, &UserPrivKey1, sharedSecret1ptr, &sharedSecret1Size, TempDHBuffptr);
    cryptocell_disable();
#endif
}

int main(void)
{
#if !defined(COSY_TEST) && !defined(TEST_STACK)
    puts("'crypto-ewsn2020_ecdh_cryptocell'");
    gpio_init(active_gpio, GPIO_OUT);
    gpio_clear(active_gpio);
    xtimer_sleep(1);

    for (int i = 0; i < ITERATIONS; i++) {
        gpio_set(active_gpio);
        _init_vars();
        gpio_clear(active_gpio);
#else
        _init_vars();
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
    printf("sizeof(UserPrivKey1): %i\n", sizeof(UserPrivKey1));
    printf("sizeof(UserPubKey1): %i\n", sizeof(UserPublKey1));
#endif
    puts("DONE");
    return 0;
}