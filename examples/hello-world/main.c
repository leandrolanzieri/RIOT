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
char teststring[] = "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Aenean";
uint8_t expected_result[] = { 0xfe, 0x54, 0xb2, 0xc7, 0xa6, 0x63, 0x3e, 0x27, 0x13, 0x58, 0x2d, 0x55, 0xbf, 0xf2, 0x18, 0x04, 0x4d, 0x1d, 0x8a, 0x78 };

size_t teststring_size = (sizeof(teststring)-1);

int main(void)
{
    puts("Hello World!");

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);
    sha1_context ctx;
    sha1_init(&ctx);
    sha1_update(&ctx, (unsigned char*)teststring, teststring_size);
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
