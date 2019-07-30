/*
 * Copyright (C) 2014 Freie Universität Berlin, Hinnerk van Bruinehsen
 *               2017 RWTH Aachen, Josua Arndt
 *               2018 Matthew Blue
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_atmega_common
 * @{
 *
 * @file
 * @brief           Implementation specific CPU configuration options
 *
 * @author          Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author          Hinnerk van Bruinehsen <h.v.bruinehsen@fu-berlin.de>
 * @author          Josua Arndt <jarndt@ias.rwth-aachen.de>
 * @author          Steffen Robertz <steffen.robertz@rwth-aachen.de>
 * @author          Matthew Blue <matthew.blue.neuro@gmail.com>
 */

#ifndef CPU_CONF_H
#define CPU_CONF_H

#include "atmega_regs_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_THREAD_EXTRA_STACKSIZE_PRINTF
#define CONFIG_THREAD_EXTRA_STACKSIZE_PRINTF    (128)
#endif

/**
 * @name           Kernel configuration
 *
 *                 Since printf seems to get memory allocated by the
 *                 linker/avr-libc the stack size tested successfully
 *                 even with pretty small stacks.
 * @{
 */
#ifndef CONFIG_THREAD_STACKSIZE_DEFAULT
#define CONFIG_THREAD_STACKSIZE_DEFAULT   (512)
#endif

/* keep CONFIG_THREAD_STACKSIZE_IDLE > CONFIG_THREAD_EXTRA_STACKSIZE_PRINTF
 * to avoid not printing of debug in interrupts
 */
#ifndef CONFIG_THREAD_STACKSIZE_IDLE
#define CONFIG_THREAD_STACKSIZE_IDLE      (128)
#endif
/** @} */

/**
 * @brief   Attribute for memory sections required by SRAM PUF
 */
#define PUF_SRAM_ATTRIBUTES __attribute__((used, section(".noinit")))

#ifdef __cplusplus
}
#endif


#endif /* CPU_CONF_H */
/** @} */
