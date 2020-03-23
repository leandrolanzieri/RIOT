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
#include "hashes/sha256.h"
#include "xtimer.h"
#include "periph/gpio.h"

uint8_t sha1_result[SHA1_DIGEST_LENGTH];
uint8_t sha256_result[SHA256_DIGEST_LENGTH];
char teststring[] = "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Aenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Donec quam felis, ultricies nec, pellentesque eu, pretium quis, sem. Nulla consequat massa quis enim. Donec pede justo, fringilla vel, aliquet nec, vulputate eget, arcu. In enim justo, rhoncus ut, imperdiet a, venenatis vitae, justo. Nullam dictum felis eu pede mollis pretium. Integer tincidunt. Cras dapibus. Vivamus elementum semper nisi. Aenean vulputate eleifend tellus. Aenean leo ligula, porttitor eu, consequat vitae, eleifend ac, enim. Aliquam lorem ante, dapibus in, viverra quis, feugiat a, tellus. Phasellus viverra nulla ut metus varius laoreet. Quisque rutrum. Aenean imperdiet. Etiam ultricies nisi vel augue";
uint8_t expected_result_sha1[] = { 0x6a, 0x7c, 0x17, 0x10, 0x4e, 0x56, 0x13, 0xa6, 0x82, 0xc5, 0x2b, 0x84, 0x54, 0xae, 0x5e, 0xbc, 0x9e, 0x8a, 0xf1, 0x67 };

uint8_t expected_result_sha256[] = { 0xfc, 0xbd, 0x7f, 0xe5, 0x12, 0x31, 0x1d, 0x1a, 0x19, 0x33, 0x87, 0x9a, 0x81, 0xe3, 0x42, 0x2e, 0x47, 0x4d, 0xf3, 0xd2, 0x46, 0xdf, 0x82, 0xdf, 0x3f, 0x63, 0x4a, 0xe1, 0x39, 0xd3, 0xb0, 0xa6 };

size_t teststring_size = (sizeof(teststring)-1);

uint32_t start, stop, t_diff;

static void sha1_test(void)
{
    gpio_init(GPIO_PIN(2,5), GPIO_OUT);
    sha1_context ctx;

    gpio_set(GPIO_PIN(2,5));
    sha1_init(&ctx);
    gpio_clear(GPIO_PIN(2,5));

    start = xtimer_now_usec();
    sha1_update(&ctx, (unsigned char*)teststring, teststring_size);
    stop = xtimer_now_usec();
    t_diff = stop - start;
    printf("Sha1 Update Time: %ld us\n", t_diff);

    gpio_set(GPIO_PIN(2,5));
    start = xtimer_now_usec();
    sha1_final(&ctx, sha1_result);
    stop = xtimer_now_usec();
    t_diff = stop - start;
    printf("Sha1 Final Time: %ld us\n", t_diff);

    gpio_clear(GPIO_PIN(2,5));
    if (memcmp(sha1_result, expected_result_sha1, SHA1_DIGEST_LENGTH) != 0) {
        printf("SHA-1 Failure\n");

        for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
            printf("%02x ", sha1_result[i]);
        }
        printf("\n");
    }
    else {
        printf("SHA-1 Success\n");
    }
}

static void sha256_test(void)
{
    gpio_init(GPIO_PIN(2,5), GPIO_OUT);
    sha256_context_t ctx;

    gpio_set(GPIO_PIN(2,5));
    sha256_init(&ctx);
    gpio_clear(GPIO_PIN(2,5));

    start = xtimer_now_usec();
    sha256_update(&ctx, (unsigned char*)teststring, teststring_size);
    stop = xtimer_now_usec();
    t_diff = stop - start;
    printf("Sha256 Update Time: %ld us\n", t_diff);

    gpio_set(GPIO_PIN(2,5));
    start = xtimer_now_usec();
    sha256_final(&ctx, sha256_result);
    stop = xtimer_now_usec();
    t_diff = stop - start;
    printf("Sha256 Final Time: %ld us\n", t_diff);

    gpio_clear(GPIO_PIN(2,5));
    if (memcmp(sha256_result, expected_result_sha256, SHA256_DIGEST_LENGTH) != 0) {
        printf("SHA-256 Failure\n");

        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            printf("%02x ", sha256_result[i]);
        }
        printf("\n");
    }
    else {
        printf("SHA-256 Success\n");
    }
}

int main(void)
{
    puts("Hello World!");
    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);
    sha1_test();
    sha256_test();
    return 0;
}
