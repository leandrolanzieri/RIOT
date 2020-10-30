/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Example to compare RIOT SHA256 and ATECC608A with two contexts
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 * @author      Lena Boeckmann <lena.boeckmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "kernel_defines.h"

#include "hashes/sha256.h"
#include "atca.h"
#include "atca_util.h"
#include "atca_params.h"

#include "xtimer.h"
#include "periph/i2c.h"

#define SHA256_HASH_SIZE (32)

uint8_t teststring[] =  "This is a teststring fore sha256";
uint8_t teststring2[] = "einealtedamegehthonigessenxxxxxx";
uint16_t test_string_size = (sizeof(teststring) - 1);   /* -1 to ignore \0 */

uint8_t expected[] =
{
0x65, 0x0C, 0x3A, 0xC7, 0xF9, 0x33, 0x17, 0xD3,
0x96, 0x31, 0xD3, 0xF5, 0xC5, 0x5B, 0x0A, 0x1E,
0x96, 0x68, 0x04, 0xE2, 0x73, 0xC3, 0x8F, 0x93,
0x9C, 0xB1, 0x45, 0x4D, 0xC2, 0x69, 0x7D, 0x20
};

/**
 * Function to call RIOT software implementation of SHA-256
 */
static int test_2_contexts(void)
{
    int ret;
    sha256_context_t ctx, ctx2;
    uint8_t res[SHA256_HASH_SIZE];
    memset(res, 0, SHA256_HASH_SIZE);

    // init first SW context
    sha256_init(&ctx);
    sha256_update(&ctx, (void *)teststring, sizeof(teststring)-1);

    // init second SW context
    sha256_init(&ctx2);
    sha256_update(&ctx2, (void *)teststring2, sizeof(teststring2)-1);

    // finalize first operation and compare with expected result
    sha256_final(&ctx, res);

    if (memcmp(expected, res, SHA256_HASH_SIZE) == 0) {
        puts("RIOT SHA SW 2 contexts OK");
    }
    else {
        puts("RIOT SHA SW 2 contexts SHICE");
    }

    // finalize second operation and keep digest in "res"
    memset(res, 0, SHA256_HASH_SIZE);
    sha256_final(&ctx2, res);

    /* ATA */
    atca_sha256_ctx_t actx, actx2;
    uint8_t ares[SHA256_HASH_SIZE];
    memset(ares, 0, SHA256_HASH_SIZE);
    uint8_t context[SHA_CONTEXT_MAX_SIZE];
    uint8_t context2[SHA_CONTEXT_MAX_SIZE];
    uint16_t context_size=sizeof(context);
    uint16_t context_size2 = sizeof(context2);

    // init first ATA context
    atcab_hw_sha2_256_init(&actx);
    atcab_hw_sha2_256_update(&actx, teststring, sizeof(teststring)-1);

    // save first context to reapply later
    atcab_sha_read_context(context, &context_size);
    // context_size = sizeof(actx.block);
    // atcab_sha_read_context(actx.block, &context_size);

    // printf("CONTEXT: \n");
    // for(int i=0;i<context_size;i++) {
    //     printf("%i ", context[i]);
    // }
    // printf("\n");

    // init second ATA context
    atcab_hw_sha2_256_init(&actx2);
    atcab_hw_sha2_256_update(&actx2, teststring2, sizeof(teststring2)-1);

    // save second context and apply first one
    atcab_sha_read_context(context2, &context_size2);
    atcab_sha_write_context(context, context_size);
    // atcab_sha_write_context(actx.block, context_size);

    // finalize first context and compare with expected value
    ret = atcab_hw_sha2_256_finish(&actx, ares);

    if (memcmp(expected, ares, SHA256_HASH_SIZE) == 0) {
        puts("ATA SHA SW 2 contexts OK");
    }
    else {
        puts("ATA SHA SW 2 contexts SHICE");
    }

    // repally second context
    memset(ares, 0, SHA256_HASH_SIZE);
    atcab_sha_write_context(context2, context_size);

    // finalize second context and compare with RIOT results
    ret = atcab_hw_sha2_256_finish(&actx2, ares);
    printf("2 atcab_hw_sha2_256_finish returned: %x\n", ret);

    if (memcmp(res, ares, SHA256_HASH_SIZE) == 0) {
        puts("RIOT & ATA digest EQUAL");
    }
    else {
        puts("RIOT & ATA digest SHOICE");
    }

    atcab_hw_sha2_256(teststring2, sizeof(teststring2) - 1, ares);
    if (memcmp(res, ares, SHA256_HASH_SIZE) == 0)
    {
        puts("ATA convenience func OK");
    }
    else
    {
        puts("ATA convenience func SHOICE");
    }
    return 0;
}

int main(void)
{
#ifdef NO_I2C_RECONF
    puts("SLOW I2C");
#else
    puts("FAST I2C");
#endif

#ifdef MODULE_PERIPH_I2C_RECONFIGURE
    puts("GPIO WAKEUP");
#else
    puts("I2C WAKEUP");
#endif

    ATCAIfaceCfg *cfg = (ATCAIfaceCfg *)&atca_params[I2C_DEV(0)];
    if (i2c_config[cfg->atcai2c.bus].speed == I2C_SPEED_NORMAL) {
        puts("I2C_SPEED_NORMAL");
    }
    if (i2c_config[cfg->atcai2c.bus].speed == I2C_SPEED_FAST) {
        puts("I2C_SPEED_FAST");
    }

    puts("TWO CONTEXTS");
    test_2_contexts();

    return 0;
}
