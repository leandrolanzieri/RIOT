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
#include "cryptocell_incl/crys_ecpki_ecdsa.h"
// #include "cryptocell_incl/crys_ecpki_dh.h"
#include "cryptocell_incl/crys_ecpki_kg.h"
#include "cryptocell_incl/crys_ecpki_domain.h"

#ifndef COSY_TEST
#include "xtimer.h"
#include "ps.h"
#include "periph/gpio.h"

gpio_t active_gpio = GPIO_PIN(1, 7);

#define ITERATIONS                  (50)
#endif

#define SHA256_DIGEST_SIZE          (32)
#define ECDSA_MESSAGE_SIZE          (127)

extern CRYS_RND_State_t*     rndState_ptr;
extern CRYS_RND_WorkBuff_t*  rndWorkBuff_ptr;

CRYS_ECPKI_UserPrivKey_t UserPrivKey;
CRYS_ECPKI_UserPublKey_t UserPublKey;
CRYS_ECDH_TempData_t signOutBuff;
CRYS_ECDSA_SignUserContext_t SignUserContext;
CRYS_ECDSA_VerifyUserContext_t VerifyUserContext;
CRYS_ECPKI_Domain_t* pDomain;
CRYS_ECDH_TempData_t* TempDHBuffptr;
CRYS_ECPKI_KG_TempData_t* TempECCKGBuffptr;
CRYS_ECDH_TempData_t TempDHBuff;
CRYS_ECPKI_KG_TempData_t TempECCKGBuff;
CRYS_ECPKI_KG_FipsContext_t FipsBuff;
SaSiRndGenerateVectWorkFunc_t rndGenerateVectFunc;
uint32_t ecdsa_sig_size = 64;

void _init_vars(void)
{
    rndGenerateVectFunc = CRYS_RND_GenerateVector;
    pDomain = (CRYS_ECPKI_Domain_t*)CRYS_ECPKI_GetEcDomain(CRYS_ECPKI_DomainID_secp256r1);
}


void _gen_keypair(void)
{
#ifndef COSY_TEST
    int ret = 0;

    cryptocell_enable();

    gpio_set(active_gpio);
    ret = CRYS_ECPKI_GenKeyPair(rndState_ptr, rndGenerateVectFunc, pDomain, &UserPrivKey, &UserPublKey, &TempECCKGBuff, &FipsBuff);
    gpio_clear(active_gpio);

    cryptocell_disable();

    if (ret != SA_SILIB_RET_OK){
        printf("CRYS_ECPKI_GenKeyPair for key pair 1 failed with 0x%x \n",ret);
        return;
    }
#else
    cryptocell_enable();
    CRYS_ECPKI_GenKeyPair(rndState_ptr, rndGenerateVectFunc, pDomain, &UserPrivKey, &UserPublKey, &TempECCKGBuff, &FipsBuff);
    cryptocell_disable();
#endif
}

void _sign_verify(void)
{
    uint8_t msg[ECDSA_MESSAGE_SIZE] = { 0x0b };

#ifndef COSY_TEST
    int ret = 0;
    /*Call CRYS_ECDSA_Sign to create signature from input buffer using created private key*/
    cryptocell_enable();

    gpio_set(active_gpio);
    ret = CRYS_ECDSA_Sign (rndState_ptr, rndGenerateVectFunc,
    &SignUserContext, &UserPrivKey, CRYS_ECPKI_HASH_SHA256_mode, msg, sizeof(msg), (uint8_t*)&signOutBuff, &ecdsa_sig_size);
    gpio_clear(active_gpio);

    cryptocell_disable();

    if (ret != SA_SILIB_RET_OK){
        printf("CRYS_ECDSA_Sign failed with 0x%x \n",ret);
        return;
    }

    cryptocell_enable();

    gpio_set(active_gpio);
    ret =  CRYS_ECDSA_Verify (&VerifyUserContext, &UserPublKey, CRYS_ECPKI_HASH_SHA256_mode, (uint8_t*)&signOutBuff, ecdsa_sig_size, msg, sizeof(msg));
    gpio_clear(active_gpio);

    cryptocell_disable();
    if (ret != SA_SILIB_RET_OK){
        printf("CRYS_ECDSA_Verify failed with 0x%x \n",ret);
        return;
    }
    puts("VALID\n");
#else
    cryptocell_enable();
    CRYS_ECDSA_Sign (rndState_ptr, rndGenerateVectFunc,
    &SignUserContext, &UserPrivKey, CRYS_ECPKI_HASH_SHA256_mode, msg, sizeof(msg), (uint8_t*)&signOutBuff, &ecdsa_sig_size);
    cryptocell_disable();

    cryptocell_enable();
    CRYS_ECDSA_Verify (&VerifyUserContext, &UserPublKey, CRYS_ECPKI_HASH_SHA256_mode, (uint8_t*)&signOutBuff, ecdsa_sig_size, msg, sizeof(msg));
    cryptocell_disable();
#endif
}

int main(void)
{
#ifndef COSY_TEST
    puts("'crypto-ewsn2020_ecdsa cryptocell'");
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
        // generate keypairs
        _gen_keypair();

        // sign data and verify with public ley
        _sign_verify();

#ifndef CORY_TEST
    }
    ps();
    printf("sizeof(UserPrivKey): %i\n", sizeof(UserPrivKey));
    printf("sizeof(UserPubKey): %i\n", sizeof(UserPublKey));
#endif
    puts("DONE");
    return 0;
}


