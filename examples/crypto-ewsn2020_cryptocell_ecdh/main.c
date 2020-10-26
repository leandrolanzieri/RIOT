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

#if !defined(TEST_MEM) && !defined(TEST_STACK)
#include <string.h>
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

#define SHARED_SECRET_LENGTH    (32)

extern CRYS_RND_State_t*     rndState_ptr;

CRYS_ECPKI_UserPrivKey_t UserPrivKey1;
CRYS_ECPKI_UserPublKey_t UserPublKey1;

CRYS_ECPKI_Domain_t* pDomain;

SaSiRndGenerateVectWorkFunc_t rndGenerateVectFunc;
uint8_t sharedSecret1ptr[SHARED_SECRET_LENGTH];
uint32_t sharedSecret1Size = SHARED_SECRET_LENGTH;

#if !defined(TEST_MEM) && !defined(TEST_STACK)
CRYS_ECPKI_UserPrivKey_t UserPrivKey2;
CRYS_ECPKI_UserPublKey_t UserPublKey2;
uint8_t sharedSecret2ptr[SHARED_SECRET_LENGTH];
uint32_t sharedSecret2Size = SHARED_SECRET_LENGTH;
#endif



void _init_vars(void)
{
    rndGenerateVectFunc = CRYS_RND_GenerateVector;

    pDomain = (CRYS_ECPKI_Domain_t*)CRYS_ECPKI_GetEcDomain(CRYS_ECPKI_DomainID_secp256r1);
}

void _gen_keypair(void)
{
    CRYS_ECPKI_KG_TempData_t* TempECCKGBuffptr;
    CRYS_ECPKI_KG_TempData_t TempECCKGBuff;
    TempECCKGBuffptr = (CRYS_ECPKI_KG_TempData_t*)&TempECCKGBuff;
    CRYS_ECPKI_KG_FipsContext_t FipsBuff;
#if !defined(TEST_MEM) && !defined(TEST_STACK)
    int ret = 0;

    _start_trigger();
    cryptocell_enable();
    ret = CRYS_ECPKI_GenKeyPair (rndState_ptr, rndGenerateVectFunc, pDomain, &UserPrivKey1, &UserPublKey1, TempECCKGBuffptr, &FipsBuff);
    cryptocell_disable();
    _stop_trigger();
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
    CRYS_ECDH_TempData_t* TempDHBuffptr;

    CRYS_ECDH_TempData_t TempDHBuff;
    TempDHBuffptr = (CRYS_ECDH_TempData_t*)&TempDHBuff;

#if !defined(TEST_MEM) && !defined(TEST_STACK)
    int ret = 0;
    /* Generating the Secret for user 1*/
    /*---------------------------------*/
    _start_trigger();
    cryptocell_enable();
    ret = CRYS_ECDH_SVDP_DH(&UserPublKey2, &UserPrivKey1, sharedSecret1ptr, &sharedSecret1Size, TempDHBuffptr);
    cryptocell_disable();
    _stop_trigger();

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
    if (memcmp(sharedSecret1ptr, sharedSecret2ptr, SHARED_SECRET_LENGTH)) {
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
#if !defined(TEST_MEM) && !defined(TEST_STACK)
    puts("'crypto-ewsn2020_ecdh_cryptocell'");
    _init_trigger();

    // xtimer_sleep(1);

    for (int i = 0; i < ITERATIONS; i++) {
        _start_trigger();
        _init_vars();
       _stop_trigger();
#else
        _init_vars();
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
    printf("sizeof(UserPrivKey1): %i\n", sizeof(UserPrivKey1));
    printf("sizeof(UserPubKey1): %i\n", sizeof(UserPublKey1));


printf("CRYS_ECDH_TempData_t: %i\n", sizeof(CRYS_ECDH_TempData_t));
printf("CRYS_ECPKI_KG_TempData_t: %i\n", sizeof(CRYS_ECPKI_KG_TempData_t));
printf("CRYS_ECPKI_KG_FipsContext_t: %i\n", sizeof(CRYS_ECPKI_KG_FipsContext_t));
#endif
    puts("DONE");
    return 0;
}