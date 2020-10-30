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
 * @brief       Example to measure processing time of ATECC608A SHA256 Operations with the shellhandler
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
#include "periph/gpio.h"

#include "hashes/sha256.h"
#include "atca.h"
#include "atca_util.h"
#include "atca_params.h"

#include "shell.h"
#include "xtimer.h"
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

/* The following functions can be used with the shell handler */

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

     /* define buffer to be used by the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];

    /* define own shell commands */
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
