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
#include "xtimer.h"
#include "em_device.h"

extern crypto_device_t crypto_devs[CRYPTO_COUNT];

#define LDMA_DESCRIPTOR_LINKREL_M2P_WORD(src, dest, count, linkjmp) \
  {                                                                 \
    .xfer =                                                         \
    {                                                               \
      .structType   = ldmaCtrlStructTypeXfer,                       \
      .structReq    = 0,                                            \
      .xferCnt      = (count) - 1,                                  \
      .byteSwap     = 0,                                            \
      .blockSize    = ldmaCtrlBlockSizeUnit1,                       \
      .doneIfs      = 1,                                            \
      .reqMode      = ldmaCtrlReqModeBlock,                         \
      .decLoopCnt   = 0,                                            \
      .ignoreSrec   = 0,                                            \
      .srcInc       = ldmaCtrlSrcIncOne,                            \
      .size         = ldmaCtrlSizeWord,                             \
      .dstInc       = ldmaCtrlDstIncNone,                           \
      .srcAddrMode  = ldmaCtrlSrcAddrModeAbs,                       \
      .dstAddrMode  = ldmaCtrlDstAddrModeAbs,                       \
      .srcAddr      = (uint32_t)(src),                              \
      .dstAddr      = (uint32_t)(dest),                             \
      .linkMode     = ldmaLinkModeRel,                              \
      .link         = 1,                                            \
      .linkAddr     = (linkjmp) * 4                                 \
    }                                                               \
  }

/* AES Test */

#define AES_SIZE_BYTES  (16U)
#define AES_SIZE_WORDS  (AES_SIZE_BYTES/4)

static uint8_t key_0[] = {
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF
};

static uint8_t plainText[] = {
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
};

volatile uint32_t iterations = 0;
mutex_t dma_lock;
// static uint8_t cipherText[] = {
//     0x37, 0x29, 0xa3, 0x6c, 0xaf, 0xe9, 0x84, 0xff,
//     0x46, 0x22, 0x70, 0x42, 0xee, 0x24, 0x83, 0xf6,
//     0x37, 0x29, 0xa3, 0x6c, 0xaf, 0xe9, 0x84, 0xff,
//     0x46, 0x22, 0x70, 0x42, 0xee, 0x24, 0x83, 0xf6
// };
#define AES_NUMBYTES (32UL)

static uint8_t cipherData[sizeof(plainText)/sizeof(plainText[0])];

void reconfig_dma_ch_0(void)
{
    /******************************************************************************/
    /* this configures the channel 0 that takes data from DATA0 to the 'out' buffer */
    LDMA->CHEN   |= 1UL << 0;     /* enable DMA read channel */

        /* configure the channel to copy 16 bytes per transaction */
    LDMA->CH[0].CTRL  |= 15 <<_LDMA_CH_CTRL_XFERCNT_SHIFT;   /* transfer 16 units (n + 1) */
}

void config_dma_ch_0(void)
{
    LDMA->IEN    |= 1UL << 0;     /* enable DONE interrupt for read channel on LDMA peripheral */
    LDMA->IFC    |= 1UL << 0;     /* clear DONE interrupt flag of read channel */
    LDMA->REQDIS = 0;             /* enable DMA requests from peripherals */

    /* configure the DMA channel */
    LDMA->CH[0].CFG = LDMA_CH_CFG_DSTINCSIGN_POSITIVE | LDMA_CH_CFG_SRCINCSIGN_POSITIVE; /* increment destination address */

    /* configure CRYPTO0 DATA0 RD as request signal */
    LDMA->CH[0].REQSEL = LDMA_CH_REQSEL_SIGSEL_CRYPTO0DATA0RD | LDMA_CH_REQSEL_SOURCESEL_CRYPTO0;

    /* configure the channel to copy 16 bytes per transaction */
    LDMA->CH[0].CTRL  =   LDMA_CH_CTRL_DSTMODE_ABSOLUTE     /* destination address is absolute */
                        | LDMA_CH_CTRL_SRCMODE_ABSOLUTE     /* source address is absolute */
                        | LDMA_CH_CTRL_DSTINC_ONE           /* increment destination address by one units after each write */
                        | LDMA_CH_CTRL_SIZE_BYTE            /* each unit transfer is a byte */
                        | LDMA_CH_CTRL_SRCINC_NONE          /* do not increment the source address */
                        //| 15 <<_LDMA_CH_CTRL_XFERCNT_SHIFT   /* transfer 16 units (n + 1) */
                        | LDMA_CH_CTRL_BLOCKSIZE_ALL
                        | LDMA_CH_CTRL_REQMODE_ALL;         /* one transfer request transfers all units defined in XFERCNT */

    LDMA->CH[0].DST = (uint32_t)cipherData;        /* set destination address to the output buffer */
    LDMA->CH[0].SRC = (uint32_t)&(CRYPTO0->DATA0BYTE); /* set source address to the DATA0 register of the used CRYPTO device */
    reconfig_dma_ch_0();
}

void reconfig_dma_ch_1(void)
{

    LDMA->CHEN   |= 1UL << 1;    /* enable DMA write channel */

    /* configure the channel to copy 16 bytes per transaction */
    LDMA->CH[1].CTRL  |= 15 <<_LDMA_CH_CTRL_XFERCNT_SHIFT;   /* transfer 16 units (n + 1) */
}

void config_dma_ch_1(void)
{
    /******************************************************************************/
    /* this configures the channel 1 that takes data from the 'in' buffer to DATA0 */
    LDMA->IEN    |= 1UL << 1;    /* enable DONE interrupt for write channel on LDMA peripheral */
    LDMA->IFC    |= 1UL << 1;    /* clear DONE interrupt flag of write channel */

    /* configure the DMA channel */
    LDMA->CH[1].CFG = LDMA_CH_CFG_SRCINCSIGN_POSITIVE | LDMA_CH_CFG_DSTINCSIGN_POSITIVE; /* increment source address */
    /* configure CRYPTO0 DATA0 WR as request signal */
    LDMA->CH[1].REQSEL = LDMA_CH_REQSEL_SIGSEL_CRYPTO0DATA0WR | LDMA_CH_REQSEL_SOURCESEL_CRYPTO0;

    /* configure the channel to copy 16 bytes per transaction */
    LDMA->CH[1].CTRL  =   LDMA_CH_CTRL_DSTMODE_ABSOLUTE     /* destination address is absolute */
                        | LDMA_CH_CTRL_SRCMODE_ABSOLUTE     /* source address is absolute */
                        | LDMA_CH_CTRL_SIZE_BYTE            /* each unit transfer is a word */
                        | LDMA_CH_CTRL_SRCINC_ONE           /* increment source address by one units after each write */
                        | LDMA_CH_CTRL_DSTINC_NONE          /* do not increment the destination address */
                        //| 15 <<_LDMA_CH_CTRL_XFERCNT_SHIFT   /* transfer 16 units (n + 1) */
                        | LDMA_CH_CTRL_BLOCKSIZE_ALL
                        | LDMA_CH_CTRL_REQMODE_ALL;         /* one transfer request transfers all units defined in XFERCNT */

    LDMA->CH[1].SRC = (uint32_t)plainText;        /* set source address to the input buffer */
    LDMA->CH[1].DST = (uint32_t)&(CRYPTO0->DATA0BYTE); /* set destination address to the DATA0 register of the used CRYPTO device */
    reconfig_dma_ch_1();
}

void trigger_crypto(void)
{
    CRYPTO0->CMD |= CRYPTO_CMD_SEQSTART;
}

void config_crypto(void)
{
    /******************************************************************************/
    /* configure the CRYPTO device */
    CRYPTO0->CTRL =   CRYPTO_CTRL_DMA0RSEL_DATA0 /* use DATA0 register for DMA0RD DMA requests */
                    | CRYPTO_CTRL_DMA0MODE_LENLIMITBYTE  /* read the full target register on every DMA transaction */
                    | CRYPTO_CTRL_AES_AES128;    /* use AES-128 mode */

    CRYPTO0->IEN |= CRYPTO_IEN_SEQDONE;

    CRYPTO0->SEQCTRL =   CRYPTO_SEQCTRL_BLOCKSIZE_16BYTES
                       | 16UL; /* NOTE: we want to handle 32 bytes in total */

    /* load a sequence of commands */
    CRYPTO_SEQ_LOAD_3(CRYPTO0,
                     CRYPTO_CMD_INSTR_DMA0TODATA,
                     CRYPTO_CMD_INSTR_AESENC,      /* encrypt with AES */
                     CRYPTO_CMD_INSTR_DATATODMA0); /* move from configured DATA register to DMA, request DMA0RD signal */

    trigger_crypto();
}

int main(void)
{
    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);
    puts("=== Test AES on CRYPTO ===");
    printf("-> Number of bytes: %ld\n", AES_NUMBYTES);

    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(cmuClock_CRYPTO0, true);

    NVIC_ClearPendingIRQ(CRYPTO0_IRQn);
    NVIC_EnableIRQ(CRYPTO0_IRQn);

    /* set the key in the CRYPTO */
    /* when the key is not word-aligned we need to copy it to a buffer */
    /* it could be checked but now not only for simplicity */
    uint32_t key[AES_SIZE_WORDS];
    memcpy(key, key_0, AES_SIZE_BYTES);

    mutex_init(&dma_lock);
    mutex_lock(&dma_lock);

    /* copy the key to the crypto buffer */
    for (unsigned i = 0; i < 4; i++) {
        CRYPTO0->KEYBUF = key[i];
    }

    LDMA_Init_t ldma_init = LDMA_INIT_DEFAULT;
    LDMA_Init(&ldma_init);

    config_dma_ch_0();
    config_dma_ch_1();
    config_crypto();

    /* wait for instruction, sequence and DMA completion */
    // while ((CRYPTO0->STATUS &(CRYPTO_STATUS_INSTRRUNNING
    //                         | CRYPTO_STATUS_SEQRUNNING
    //                         | CRYPTO_STATUS_DMAACTIVE)) != 0) { }
    puts("locking");
    mutex_lock(&dma_lock);

    puts("-> Done");

    for (unsigned i = 0; i < sizeof(cipherData)/sizeof(cipherData[0]); i++) {
        printf("[0x%x], ", cipherData[i]);
    }

    puts("");

    printf("LDMA Channel 0 dst addr: %p\n", (void *)LDMA->CH[0].DST);
    printf("LDMA Channel 1 src addr: %p\n", (void *)LDMA->CH[1].SRC);
    printf("Input buff addr: %p\n", plainText);
    printf("beginning of second half of input: %p\n", &plainText[16]);
    printf("Output buff addrs: %p\n", cipherData);
    printf("beginning of second half of output: %p\n", &cipherData[16]);

}

void isr_ldma(void)
{
    /* clean interrupt flags */
    LDMA->IFC = 0xFFFFFFFF;
    puts("!!l");
}

void isr_crypto0(void)
{
    /* clean interrupt flags */
    CRYPTO0->IFC = 0xFFFFFFFF;
    iterations++;
    puts("c!");
    if (iterations >= 2) {
        mutex_unlock(&dma_lock);
    }
    else {
        reconfig_dma_ch_0();
        reconfig_dma_ch_1();
        trigger_crypto();
    }
}

void isr_crypto1(void)
{
    /* clean interrupt flags */
    CRYPTO1->IFC = 0xFFFFFFFF;
}
