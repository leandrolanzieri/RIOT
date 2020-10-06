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
#include "kernel_defines.h"
#include "crypto_util.h"
#include "mutex.h"
#include "em_ldma.h"
#include "em_device.h"

extern crypto_device_t crypto_devs[CRYPTO_COUNT];

/* AES Test */

#define AES_SIZE_BYTES  (16U)
#define AES_SIZE_WORDS  (AES_SIZE_BYTES/4)

static uint8_t key_0[] = {
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF
};

#if IS_ACTIVE(CONFIG_AES_NUMBYTES_16)
static uint8_t plainText[] = {
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7
};
static uint8_t cipherText[] = {
    0x37, 0x29, 0xa3, 0x6c, 0xaf, 0xe9, 0x84, 0xff,
    0x46, 0x22, 0x70, 0x42, 0xee, 0x24, 0x83, 0xf6
};
#define AES_NUMBYTES (16UL)
#else
static uint8_t plainText[] = {
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7
};
static uint8_t cipherText[] = {
    0x37, 0x29, 0xa3, 0x6c, 0xaf, 0xe9, 0x84, 0xff,
    0x46, 0x22, 0x70, 0x42, 0xee, 0x24, 0x83, 0xf6,
    0x37, 0x29, 0xa3, 0x6c, 0xaf, 0xe9, 0x84, 0xff,
    0x46, 0x22, 0x70, 0x42, 0xee, 0x24, 0x83, 0xf6
};
#define AES_NUMBYTES (32UL)
#endif

static uint8_t cipherData[AES_NUMBYTES];

int is_result_expected(uint8_t *result, uint8_t *expected, size_t bytes)
{
    for (unsigned i = 0; i < bytes; i++) {
        if (result[0] != expected[0]) {
            return 0;
        }
    }
    return 1;
}

static inline uint32_t get_dma_source(CRYPTO_TypeDef *dev)
{
    if (dev == CRYPTO0) {
        return LDMA_CH_REQSEL_SOURCESEL_CRYPTO0;
    }
    else {
        return LDMA_CH_REQSEL_SOURCESEL_CRYPTO1;
    }
}

static inline uint32_t get_dma_source_data0_signal(CRYPTO_TypeDef *dev)
{
    if (dev == CRYPTO0) {
        return LDMA_CH_REQSEL_SIGSEL_CRYPTO0DATA0RD;
    }
    else {
        return LDMA_CH_REQSEL_SIGSEL_CRYPTO1DATA0RD;
    }
}

static inline uint32_t get_dma_source_and_signal_data0_read(CRYPTO_TypeDef *dev)
{
    if (dev == CRYPTO0) {
        return LDMA_CH_REQSEL_SIGSEL_CRYPTO0DATA0RD | LDMA_CH_REQSEL_SOURCESEL_CRYPTO0;
    }
    else {
        return LDMA_CH_REQSEL_SIGSEL_CRYPTO1DATA0RD | LDMA_CH_REQSEL_SOURCESEL_CRYPTO1;
    }
}

static inline uint32_t get_dma_source_and_signal_data0_write(CRYPTO_TypeDef *dev)
{
    if (dev == CRYPTO0) {
        return LDMA_CH_REQSEL_SIGSEL_CRYPTO0DATA0WR | LDMA_CH_REQSEL_SOURCESEL_CRYPTO0;
    }
    else {
        return LDMA_CH_REQSEL_SIGSEL_CRYPTO1DATA0WR | LDMA_CH_REQSEL_SOURCESEL_CRYPTO1;
    }
}

static void init_crypto_read_dma_channel(crypto_device_t* crypto, uint8_t *out)
{
    LDMA->IEN    = 1UL << (crypto->read_dma_ch);    /* enable DONE interrupt for read channel on LDMA peripheral */
    LDMA->IFC    = 1UL << (crypto->read_dma_ch);    /* clear DONE interrupt flag of read channel */
    LDMA->CHEN  |= 1UL << (crypto->read_dma_ch);    /* enable DMA read channel */
    LDMA->REQDIS = 0;                              /* enable DMA requests from peripherals */

    /* configure the DMA channel */
    LDMA->CH[crypto->read_dma_ch].CFG = LDMA_CH_CFG_DSTINCSIGN_POSITIVE; /* increment destination address */
    LDMA->CH[crypto->read_dma_ch].REQSEL = get_dma_source_and_signal_data0_read(crypto->dev);
    LDMA->CH[crypto->read_dma_ch].CTRL  =   LDMA_CH_CTRL_DSTMODE_ABSOLUTE     /* destination address is absolute */
                        | LDMA_CH_CTRL_DSTINC_ONE           /* increment destination address by one units after each write */
                        | LDMA_CH_CTRL_SIZE_BYTE            /* each unit transfer is a byte */
                        | LDMA_CH_CTRL_SRCINC_NONE          /* do not increment the source address */
                        | 15 <<_LDMA_CH_CTRL_XFERCNT_SHIFT  /* transfer 16 units (n + 1) */
                        | LDMA_CH_CTRL_REQMODE_ALL;         /* one transfer request transfers all units defined in XFERCNT */

    LDMA->CH[crypto->read_dma_ch].DST = (uint32_t)out;        /* set destination address to the output buffer */
    LDMA->CH[crypto->read_dma_ch].SRC = (uint32_t)&(crypto->dev->DATA0BYTE); /* set source address to the DATA0 register of the used CRYPTO device */
}

static void init_crypto_write_dma_channel(crypto_device_t *crypto, uint8_t *in)
{
    LDMA->IEN    = 1UL << (crypto->write_dma_ch);    /* enable DONE interrupt for write channel on LDMA peripheral */
    LDMA->IFC    = 1UL << (crypto->write_dma_ch);    /* clear DONE interrupt flag of write channel */
    LDMA->CHEN  |= 1UL << (crypto->write_dma_ch);    /* enable DMA write channel */
    LDMA->REQDIS = 0;                                /* enable DMA requests from peripherals */

    /* configure the DMA channel */
    LDMA->CH[crypto->write_dma_ch].CFG = LDMA_CH_CFG_SRCINCSIGN_POSITIVE; /* increment source address */
    LDMA->CH[crypto->write_dma_ch].REQSEL = get_dma_source_and_signal_data0_write(crypto->dev);
    LDMA->CH[crypto->write_dma_ch].CTRL  =    LDMA_CH_CTRL_DSTMODE_ABSOLUTE     /* destination address is absolute */
                                            | LDMA_CH_CTRL_SRCINC_ONE           /* increment source address by one units after each write */
                                            | LDMA_CH_CTRL_SIZE_BYTE            /* each unit transfer is a byte */
                                            | LDMA_CH_CTRL_DSTINC_NONE          /* do not increment the destination address */
                                            | 15 <<_LDMA_CH_CTRL_XFERCNT_SHIFT  /* transfer 16 units (n + 1) */
                                            | LDMA_CH_CTRL_REQMODE_ALL;         /* one transfer request transfers all units defined in XFERCNT */

    LDMA->CH[crypto->write_dma_ch].SRC = (uint32_t)in;        /* set source address to the input buffer */
    LDMA->CH[crypto->write_dma_ch].DST = (uint32_t)&(crypto->dev->DATA0BYTE); /* set destination address to the DATA0 register of the used CRYPTO device */
}

static void init_crypto_aes_128_encrypt(crypto_device_t* crypto, size_t length)
{
    /* configure the CRYPTO device */
    crypto->dev->CTRL =   CRYPTO_CTRL_DMA0RSEL_DATA0 /* use DATA0 register for DMA0RD DMA requests */
                        | CRYPTO_CTRL_DMA0MODE_FULLBYTE  /* read the full target register on every DMA transaction */
                        | CRYPTO_CTRL_AES_AES128;    /* use AES-128 mode */

    crypto->dev->WAC = 0;
    crypto->dev->SEQCTRL= length; /* NOTE: we want to handle 16 bytes in total */

    /* load a sequence of commands */
    crypto->dev->IEN |= CRYPTO_IEN_SEQDONE;

    CRYPTO_EXECUTE_3(crypto->dev,
                      CRYPTO_CMD_INSTR_DMA0TODATA,
                      CRYPTO_CMD_INSTR_AESENC,      /* encrypt with AES */
                      CRYPTO_CMD_INSTR_DATATODMA0); /* move from configured DATA register to DMA, request DMA0RD signal */
}

int main(void)
{
    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);
    puts("=== Test AES on CRYPTO ===");
    printf("-> Number of bytes: %ld\n", AES_NUMBYTES);

    crypto_device_t *crypto = crypto_acquire_dev();

    /* set the key in the CRYPTO */
    /* when the key is not word-aligned we need to copy it to a buffer */
    /* it could be checked but now not only for simplicity */
    uint32_t key[AES_SIZE_WORDS];
    memcpy(key, key_0, AES_SIZE_BYTES);

    for (unsigned i = 0; i < AES_SIZE_WORDS; i++) {
        crypto->dev->KEYBUF = key[i];
    }

    if (IS_ACTIVE(CONFIG_AES_WITH_DMA)) {
        puts("== Testing AES with DMA ==");

        LDMA_Init_t ldma_init = LDMA_INIT_DEFAULT;
        LDMA_Init(&ldma_init);

        init_crypto_read_dma_channel(crypto, cipherData);
        init_crypto_write_dma_channel(crypto, plainText);
        init_crypto_aes_128_encrypt(crypto, AES_NUMBYTES);

        /* wait for instruction, sequence and DMA completion */
        while ((crypto->dev->STATUS &(CRYPTO_STATUS_INSTRRUNNING
                                    | CRYPTO_STATUS_SEQRUNNING
                                    | CRYPTO_STATUS_DMAACTIVE)) != 0) { }
    }
    else {
        puts("== Testing AES without DMA ==");


        /* load the plain text in the CRYPTO */
        for (int i = 0; i < 16; i++) {
            crypto->dev->DATA0BYTE = plainText[i];
        }

        crypto->dev->CTRL = CRYPTO_CTRL_AES_AES128; /* use AES-128 mode */

        /* trigger AES encryption */
        crypto->dev->CMD = CRYPTO_CMD_INSTR_AESENC;

        /* Wait for completion */
        while ((crypto->dev->IF & CRYPTO_IF_INSTRDONE) == 0UL) { }
        crypto->dev->IFC = CRYPTO_IF_INSTRDONE;

        // TODO: Here goes a loop!
        for (unsigned i = 0; i < AES_NUMBYTES; i++) {
            cipherData[i] = crypto->dev->DATA0BYTE;
        }
    }

    puts("-> Done");

    if (is_result_expected(cipherData, cipherText, AES_NUMBYTES)) {
            puts("=> CORRECT");
    }
    else {
        puts("=> INCORRECT!");
    }

    for (unsigned i = 0; i < AES_NUMBYTES; i++) {
            printf("[0x%x], ", cipherData[i]);
    }

    puts("");

}

void isr_ldma(void)
{
    /* clean interrupt flags */
    LDMA->IFC = 0xFFFFFFFF;
}

void isr_crypto0(void)
{
    /* clean interrupt flags */
    CRYPTO0->IFC = 0xFFFFFFFF;
}

void isr_crypto1(void)
{
    /* clean interrupt flags */
    CRYPTO1->IFC = 0xFFFFFFFF;
}
