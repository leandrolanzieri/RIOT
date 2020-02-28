/*
 * Copyright (C) 2014 Freie Universität Berlin
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
 * @brief       Hello World application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "hashes/sha1.h"

uint8_t result[SHA1_DIGEST_LENGTH];
char teststring[] = "Im dichten Fichtendickicht picken zwei Finken tuechtig";
uint8_t expected_result[] = { 0xb7, 0xfc, 0x27, 0x42, 0x78, 0x62, 0xae, 0x3b, 0x07, 0x4e, 0x90, 0x21, 0xec, 0x78, 0x92, 0xec, 0x12, 0xac, 0x5e, 0x05 };

size_t teststring_size = (sizeof(teststring)-1);

int main(void)
{
    puts("Hello World!");

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);
    sha1_context ctx;
    sha1_init(&ctx);
    sha1_update(&ctx, teststring, teststring_size);
    sha1_final(&ctx, result);

    if (memcmp(result, expected_result, SHA1_DIGEST_LENGTH) != 0) {
        printf("SHA-1 Failure\n");

        for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
            printf("%02x ", result[i]);
        }
        printf("\n");
    }
    else {
        printf("SHA-1 Success\n");
    }

    return 0;
}
