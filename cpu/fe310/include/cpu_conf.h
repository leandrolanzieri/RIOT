/*
 * Copyright (C) 2017 Ken Rabold
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup         cpu_fe310
 * @{
 *
 * @file
 * @brief           CPU specific configuration options
 *
 * @author          Ken Rabold
 */

#ifndef CPU_CONF_H
#define CPU_CONF_H

/**
 * @name Configuration of default stack sizes
 * @{
 */
#ifndef CONFIG_THREAD_EXTRA_STACKSIZE_PRINTF
#define CONFIG_THREAD_EXTRA_STACKSIZE_PRINTF   (256)
#endif
#ifndef CONFIG_THREAD_STACKSIZE_DEFAULT
#define CONFIG_THREAD_STACKSIZE_DEFAULT        (1024)
#endif
#ifndef CONFIG_THREAD_STACKSIZE_IDLE
#define CONFIG_THREAD_STACKSIZE_IDLE           (256)
#endif
/** @} */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* CPU_CONF_H */
/** @} */
