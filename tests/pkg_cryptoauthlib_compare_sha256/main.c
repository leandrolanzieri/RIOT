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
 * @brief       This test was written to compare the runtime of the RIOT software
 *              implementation and the CryptoAuth hardware implementation of SHA-256.
 *
 * @author      Lena Boeckmann <lena.boeckmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "hashes/sha256.h"
#include "atca.h"
#include "atca_params.h"

#include "shell.h"
#include "xtimer.h"
#include "periph/gpio.h"
#include "periph/i2c.h"

#define SHA256_HASH_SIZE (32)

#ifndef NUM_ITER
#define NUM_ITER 10
#endif

uint32_t start, stop;

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

uint8_t result[SHA256_HASH_SIZE];                       /* +3 to fit 1 byte length and 2 bytes checksum */

atca_sha256_ctx_t ctx;


void atecc_wake(void)
{
    ATCAIfaceCfg *cfg = (ATCAIfaceCfg *)&atca_params[I2C_DEV(0)];
    uint8_t data[4] = { 0 };

#if IS_USED(MODULE_PERIPH_I2C_RECONFIGURE)
    /* switch I2C peripheral to GPIO function */
    i2c_deinit_pins(cfg->atcai2c.bus);
    gpio_init(i2c_pin_sda(cfg->atcai2c.bus), GPIO_OUT);

    /* send wake pulse of 100us (t_WOL) */
    gpio_clear(i2c_pin_sda(cfg->atcai2c.bus));
    atca_delay_us(100);

    /* reinit I2C peripheral */
    i2c_init_pins(cfg->atcai2c.bus);
#else
    /* send wake pulse by sending byte 0x00 */
    /* this requires the I2C clock to be 100kHz at a max */
    i2c_acquire(cfg->atcai2c.bus);
    i2c_write_byte(cfg->atcai2c.bus, ATCA_WAKE_ADDR, data[0], 0);
    i2c_release(cfg->atcai2c.bus);
#endif

    atca_delay_us(cfg->wake_delay);

    uint8_t retries = cfg->rx_retries;
    int status = -1;

    i2c_acquire(cfg->atcai2c.bus);
    while (retries-- > 0 && status != 0) {
        status = i2c_read_bytes(cfg->atcai2c.bus,
                                (cfg->atcai2c.slave_address >> 1),
                                &data[0], 4, 0);
    }
    i2c_release(cfg->atcai2c.bus);

    if (status != ATCA_SUCCESS) {
        printf("Communication with device failed\n");
        return;
    }

    if (hal_check_wake(data, 4)) {
        printf("Wake up failed\n");
    }
}

void atecc_idle(void)
{
    ATCAIfaceCfg *cfg = (ATCAIfaceCfg *)&atca_params[I2C_DEV(0)];
    i2c_acquire(cfg->atcai2c.bus);
    i2c_write_byte(cfg->atcai2c.bus, (cfg->atcai2c.slave_address >> 1), ATCA_IDLE_ADDR, 0);
    i2c_release(cfg->atcai2c.bus);
}

void atecc_sleep(void)
{
    ATCAIfaceCfg *cfg = (ATCAIfaceCfg *)&atca_params[I2C_DEV(0)];
    i2c_acquire(cfg->atcai2c.bus);
    i2c_write_byte(cfg->atcai2c.bus, (cfg->atcai2c.slave_address >> 1), ATCA_SLEEP_ADDR, 0);
    i2c_release(cfg->atcai2c.bus);
}

static void ata_with_ctx_save(void)
{

    atca_sha256_ctx_t ctx;
    uint8_t res[SHA256_HASH_SIZE];
    uint8_t context[SHA_CONTEXT_MAX_SIZE];
    memset(context, 0, SHA_CONTEXT_MAX_SIZE);
    uint16_t context_size=sizeof(context);


    start = xtimer_now_usec();
#ifdef ATCA_MANUAL_ONOFF
    atecc_wake();
#endif


    for (int i = 0; i < NUM_ITER; i++) {
        // printf("LOOP #%i\n", i);
        if (atcab_hw_sha2_256_init(&ctx) != ATCA_SUCCESS) {
            puts("atcab_hw_sha2_256_initERROR");
        }
        if (atcab_sha_read_context(context, &context_size) != ATCA_SUCCESS) {
            puts("1atcab_sha_read_contextERROR");
        }

        if (atcab_sha_write_context(context, context_size) != ATCA_SUCCESS) {
        puts("atcab_sha_write_contextERROR");
        }
        if (atcab_hw_sha2_256_update(&ctx, teststring, sizeof(teststring)-1) != ATCA_SUCCESS) {
        puts("atcab_hw_sha2_256_updateERROR");
        }
        if (atcab_sha_read_context(context, &context_size) != ATCA_SUCCESS) {
        puts("2atcab_sha_read_contextERROR");
        }

        if (atcab_sha_write_context(context, context_size) != ATCA_SUCCESS) {
        puts("atcab_sha_write_contextERROR");
    }
        if (atcab_hw_sha2_256_finish(&ctx, res) != ATCA_SUCCESS) {
        puts("atcab_hw_sha2_256_finishERROR");
    }
    //     if (atcab_sha_read_context(context, &context_size) != ATCA_SUCCESS) {
    //     puts("3atcab_sha_read_contextERROR");
    // }
    }

#ifdef ATCA_MANUAL_ONOFF
    atecc_sleep();
#endif
    stop = xtimer_now_usec();
    printf("ata_with_ctx_save: %i Bytes int %"PRIu32 " us\n", (NUM_ITER*SHA256_HASH_SIZE), (stop-start));

    // if (!memcmp(expected, res, SHA256_HASH_SIZE) == 0) {
    //     puts("ata_with_ctx_save ERROR");
    // }
    // else {
    //     puts("ata_without_ctx_save alrido");
    // }
}

static void ata_without_ctx_save(void)
{
    atca_sha256_ctx_t ctx;
    uint8_t res[SHA256_HASH_SIZE];

    start = xtimer_now_usec();
#ifdef ATCA_MANUAL_ONOFF
    atecc_wake();
#endif


    for (int i = 0; i < NUM_ITER; i++) {
        // printf("LOOP #%i\n", i);
        if (atcab_hw_sha2_256_init(&ctx) != ATCA_SUCCESS) {
            puts("atcab_hw_sha2_256_initERROR");
        }
        if (atcab_hw_sha2_256_update(&ctx, teststring, sizeof(teststring)-1) != ATCA_SUCCESS) {
        puts("atcab_hw_sha2_256_updateERROR");
        }
        if (atcab_hw_sha2_256_finish(&ctx, res) != ATCA_SUCCESS) {
        puts("atcab_hw_sha2_256_finishERROR");
        }
    }

#ifdef ATCA_MANUAL_ONOFF
    atecc_sleep();
#endif

    stop = xtimer_now_usec();
    printf("ata_without_ctx_save: %i Bytes int %"PRIu32 " us\n", (NUM_ITER*SHA256_HASH_SIZE), (stop-start));
}


/**
 * Function to call RIOT software implementation of SHA-256
 */
// static int test_2_contexts(void)
// {
//     int ret;
//     sha256_context_t ctx, ctx2;
//     uint8_t res[SHA256_HASH_SIZE];
//     memset(res, 0, SHA256_HASH_SIZE);

//     // init first SW context
//     sha256_init(&ctx);
//     sha256_update(&ctx, (void *)teststring, sizeof(teststring)-1);

//     // init second SW context
//     sha256_init(&ctx2);
//     sha256_update(&ctx2, (void *)teststring2, sizeof(teststring2)-1);

//     // finalize first operation and compare with expected result
//     sha256_final(&ctx, res);

//     if (memcmp(expected, res, SHA256_HASH_SIZE) == 0) {
//         puts("RIOT SHA SW 2 contexts OK");
//     }
//     else {
//         puts("RIOT SHA SW 2 contexts SHICE");
//     }

//     // finalize second operation and keep digest in "res"
//     memset(res, 0, SHA256_HASH_SIZE);
//     sha256_final(&ctx2, res);

//     /* ATA */
//     atca_sha256_ctx_t actx, actx2;
//     uint8_t ares[SHA256_HASH_SIZE];
//     memset(ares, 0, SHA256_HASH_SIZE);
//     uint8_t context[SHA_CONTEXT_MAX_SIZE];
//     uint8_t context2[SHA_CONTEXT_MAX_SIZE];
//     uint16_t context_size=sizeof(context);
//     uint16_t context_size2 = sizeof(context2);

//     // init first ATA context
//     atcab_hw_sha2_256_init(&actx);
//     atcab_hw_sha2_256_update(&actx, teststring, sizeof(teststring)-1);

//     // save first context to reapply later
//     atcab_sha_read_context(context, &context_size);
//     // context_size = sizeof(actx.block);
//     // atcab_sha_read_context(actx.block, &context_size);

//     // printf("CONTEXT: \n");
//     // for(int i=0;i<context_size;i++) {
//     //     printf("%i ", context[i]);
//     // }
//     // printf("\n");

//     // init second ATA context
//     atcab_hw_sha2_256_init(&actx2);
//     atcab_hw_sha2_256_update(&actx2, teststring2, sizeof(teststring2)-1);

//     // save second context and apply first one
//     atcab_sha_read_context(context2, &context_size2);
//     atcab_sha_write_context(context, context_size);
//     // atcab_sha_write_context(actx.block, context_size);

//     // finalize first context and compare with expected value
//     ret = atcab_hw_sha2_256_finish(&actx, ares);

//     if (memcmp(expected, ares, SHA256_HASH_SIZE) == 0) {
//         puts("ATA SHA SW 2 contexts OK");
//     }
//     else {
//         puts("ATA SHA SW 2 contexts SHICE");
//     }

//     // repally second context
//     memset(ares, 0, SHA256_HASH_SIZE);
//     atcab_sha_write_context(context2, context_size);

//     // finalize second context and compare with RIOT results
//     ret = atcab_hw_sha2_256_finish(&actx2, ares);
//     printf("2 atcab_hw_sha2_256_finish returned: %x\n", ret);

//     if (memcmp(res, ares, SHA256_HASH_SIZE) == 0) {
//         puts("RIOT & ATA digest EQUAL");
//     }
//     else {
//         puts("RIOT & ATA digest SHOICE");
//     }

//     atcab_hw_sha2_256(teststring2, sizeof(teststring2) - 1, ares);
//     if (memcmp(res, ares, SHA256_HASH_SIZE) == 0)
//     {
//         puts("ATA convenience func OK");
//     }
//     else
//     {
//         puts("ATA convenience func SHOICE");
//     }
//     return 0;
// }


/**
 * Function to call CryptoAuth hardware implementation of SHA-256
 */
static int test_atca_sha(uint8_t *teststring, uint16_t len, uint8_t *expected,
                         uint8_t *result)
{
    atcab_sha_start();
    atcab_sha_end(result, len, teststring);
    return memcmp(expected, result, SHA256_HASH_SIZE);
}

static int ata_sha_init(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    start = xtimer_now_usec();
    atcab_hw_sha2_256_init(&ctx);
    printf("atcab_hw_sha2_256_init: %"PRIu32"\n", xtimer_now_usec()-start);
    return 0;
}


static int ata_sha_update(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    start = xtimer_now_usec();
    atcab_hw_sha2_256_update(&ctx, teststring, test_string_size);
    printf("atcab_hw_sha2_256_update: %"PRIu32"\n", xtimer_now_usec()-start);
    return 0;
}

static int ata_sha_finish(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    start = xtimer_now_usec();
    atcab_hw_sha2_256_finish(&ctx, result);
    printf("atcab_hw_sha2_256_finish: %"PRIu32"\n", xtimer_now_usec()-start);
    return 0;
}

static int ata_wake(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    start = xtimer_now_usec();
    atecc_wake();
    printf("atecc_wake: %"PRIu32";\n", xtimer_now_usec()-start);
    return 0;
}

static int ata_serial(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    uint8_t sn[ATCA_SERIAL_NUM_SIZE];
    ATCA_STATUS status;

    if ((status = atcab_read_serial_number(sn)) != ATCA_SUCCESS)
    {
        puts("error reading SN");
        return -1;
    }
    printf("SERIAl NO: ");
    for (unsigned i=0;i<sizeof(sn);i++) {
        printf("0x%x ", sn[i]);
    }
    printf("\n");
    return 0;
}

static int ata_idle(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    start = xtimer_now_usec();
    atecc_idle();
    printf("atecc_idle: %"PRIu32";\n", xtimer_now_usec()-start);
    return 0;
}
static int ata_sleep(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    start = xtimer_now_usec();
    atecc_sleep();
    printf("atecc_sleep: %"PRIu32";\n", xtimer_now_usec()-start);

    return 0;
}
static const shell_command_t shell_commands[] = {
    { "asi", "ata sha init", ata_sha_init },
    { "asu", "ata sha update", ata_sha_update },
    { "asf", "ata sha finish", ata_sha_finish },
    { "awake", "ata wake up", ata_wake },
    { "aidle", "ata idle", ata_idle },
    { "asleep", "ata sleep", ata_sleep },
    { "aserial", "ata serial", ata_serial },
    { NULL, NULL, NULL }
};

int main(void)
{

    memset(result, 0, SHA256_HASH_SIZE);                    /* alles in result auf 0 setzen */

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

#ifdef ATCA_MANUAL_ONOFF
    // sleep initially
    atecc_sleep();
    puts("ATCA_MANUAL_ON");
#else
    puts("ATCA_MANUAL_OFF");
#endif

    if (test_atca_sha(teststring, test_string_size, expected, result) == 0) {
        printf("ATCA SHA256: Success\n");
    }
    else {
        printf("ATCA SHA256: Failure.\n");
    }
    ata_with_ctx_save();
    ata_without_ctx_save();

    // test_2_contexts();


     /* define buffer to be used by the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];

    /* define own shell commands */
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
