/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gnrc_ipv6_nib_conf  GNRC IPv6 NIB compile configurations
 * @ingroup     net_gnrc_ipv6_nib
 * @ingroup     net_gnrc_conf
 * @brief       Configuration macros for neighbor information base
 * @{
 *
 * @file
 * @brief       Configuration macro definitions for neighbor information base
 *
 * @author      Martine Lenders <mlenders@inf.fu-berlin.de>
 */
#ifndef NET_GNRC_IPV6_NIB_CONF_H
#define NET_GNRC_IPV6_NIB_CONF_H

#include "kernel_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/**
 * @name    Compile flags
 * @brief   Compile flags to (de-)activate certain features for NIB
 * @{
 */
/**
 * @brief   enable features for 6Lo border router
 */
#ifndef GNRC_IPV6_NIB_CONF_6LBR
#ifdef MODULE_GNRC_IPV6_NIB_6LBR
#define GNRC_IPV6_NIB_CONF_6LBR         1
#else
#define GNRC_IPV6_NIB_CONF_6LBR         0
#endif
#endif /* GNRC_IPV6_NIB_CONF_6LBR */

/**
 * @brief handle NDP messages according for stateless address
 *        auto-configuration (if activated on interface).
 * @see [RFC 4862](https://tools.ietf.org/html/rfc4862)
 */
#ifndef GNRC_IPV6_NIB_CONF_SLAAC
#if IS_USED(MODULE_GNRC_IPV6_NIB_6LBR)
#define GNRC_IPV6_NIB_CONF_SLAAC        1
#elif !IS_USED(MODULE_GNRC_IPV6_NIB_6LR) || !IS_USED(MODULE_GNRC_IPV6_NIB_6LN)
#define GNRC_IPV6_NIB_CONF_SLAAC        0
#else
#define GNRC_IPV6_NIB_CONF_SLAAC        1
#endif
#endif /* GNRC_IPV6_NIB_CONF_SLAAC */

/**
 * @brief   Number of entries in NIB
 *
 * @attention   This number has direct influence on the maximum number of
 *              neighbors and duplicate address detection table entries
 */
#ifndef GNRC_IPV6_NIB_NUMOF
#if IS_USED(MODULE_GNRC_IPV6_NIB_6LBR)
#define GNRC_IPV6_NIB_NUMOF             16
#elif IS_USED(MODULE_GNRC_IPV6_NIB_6LN) && !IS_ACTIVE(GNRC_IPV6_NIB_CONF_6LR)
/* only needs to store default router */
#define GNRC_IPV6_NIB_NUMOF             1
#else
#define GNRC_IPV6_NIB_NUMOF             4
#endif
#endif /* GNRC_IPV6_NIB_NUMOF */

/**
 * @brief   enable features for 6Lo router
 */
#ifndef GNRC_IPV6_NIB_CONF_6LR
#if IS_USED(MODULE_GNRC_IPV6_NIB_6LR) || IS_ACTIVE(GNRC_IPV6_NIB_CONF_6LBR)
#define GNRC_IPV6_NIB_CONF_6LR          1
#else
#define GNRC_IPV6_NIB_CONF_6LR          0
#endif
#endif /* GNRC_IPV6_NIB_CONF_6LR */

/**
 * @brief   enable features for 6Lo node
 */
#ifndef GNRC_IPV6_NIB_CONF_6LN
#if IS_USED(MODULE_GNRC_IPV6_NIB_6LN) || IS_ACTIVE(GNRC_IPV6_NIB_CONF_6LR)
#define GNRC_IPV6_NIB_CONF_6LN          1
#else
#define GNRC_IPV6_NIB_CONF_6LN          0
#endif
#endif /* GNRC_IPV6_NIB_CONF_6LN */

/**
 * @brief    queue packets for address resolution
 */
#ifndef GNRC_IPV6_NIB_CONF_QUEUE_PKT
#if IS_USED(MODULE_GNRC_IPV6_NIB_6LN) || IS_ACTIVE(GNRC_IPV6_NIB_CONF_6LN)
#define GNRC_IPV6_NIB_CONF_QUEUE_PKT    0
#else
#define GNRC_IPV6_NIB_CONF_QUEUE_PKT    1
#endif
#endif /* GNRC_IPV6_NIB_CONF_QUEUE_PKT */

/**
 * @brief   (de-)activate NDP address resolution state-machine
 */
#ifndef GNRC_IPV6_NIB_CONF_ARSM
#if IS_USED(MODULE_GNRC_IPV6_NIB_6LN) && !IS_ACTIVE(GNRC_IPV6_NIB_CONF_6LR)
#define GNRC_IPV6_NIB_CONF_ARSM         0
#else
#define GNRC_IPV6_NIB_CONF_ARSM         1
#endif
#endif /* GNRC_IPV6_NIB_CONF_ARSM */

/**
 * @brief   enable features for IPv6 routers
 */
/* When using this module router features are not optional */
#if IS_USED(MODULE_GNRC_IPV6_NIB_ROUTER)
#define GNRC_IPV6_NIB_CONF_ROUTER       1
#endif

#ifndef GNRC_IPV6_NIB_CONF_ROUTER
#if IS_ACTIVE(GNRC_IPV6_NIB_CONF_6LR)
#define GNRC_IPV6_NIB_CONF_ROUTER       1
#else
#define GNRC_IPV6_NIB_CONF_ROUTER       0
#endif
#endif /* GNRC_IPV6_NIB_CONF_ROUTER */

/**
 * @brief   Support for DNS configuration options
 *
 * @see [RFC 8106](https://tools.ietf.org/html/rfc8106)
 */
#ifdef IS_USED(MODULE_GNRC_IPV6_NIB_DNS)
#define GNRC_IPV6_NIB_CONF_DNS          1
#endif

#ifndef GNRC_IPV6_NIB_CONF_DNS
#define GNRC_IPV6_NIB_CONF_DNS          0
#endif /* GNRC_IPV6_NIB_CONF_DNS */

/**
 * @brief   enable features for IPv6 routers
 */
#ifndef GNRC_IPV6_NIB_CONF_ROUTER
#if IS_ACTIVE(GNRC_IPV6_NIB_CONF_6LR)
#define GNRC_IPV6_NIB_CONF_ROUTER       1
#else
#define GNRC_IPV6_NIB_CONF_ROUTER       0
#endif
#endif /* GNRC_IPV6_NIB_CONF_ROUTER */

/**
 * @brief    (de-)activate router advertising at interface start-up
 */
#ifndef GNRC_IPV6_NIB_CONF_ADV_ROUTER
#if IS_ACTIVE(GNRC_IPV6_NIB_CONF_ROUTER) && \
    (!IS_ACTIVE(GNRC_IPV6_NIB_CONF_6LR) || IS_ACTIVE(GNRC_IPV6_NIB_CONF_6LBR))
#define GNRC_IPV6_NIB_CONF_ADV_ROUTER   1
#else
#define GNRC_IPV6_NIB_CONF_ADV_ROUTER   0
#endif
#endif /* GNRC_IPV6_NIB_CONF_ADV_ROUTER */

/**
 * @brief    handle Redirect Messages
 */
#ifndef GNRC_IPV6_NIB_CONF_REDIRECT
#define GNRC_IPV6_NIB_CONF_REDIRECT     0
#endif

/**
 * @brief   (de-)activate destination cache
 */
#ifndef GNRC_IPV6_NIB_CONF_DC
#if IS_ACTIVE(GNRC_IPV6_NIB_CONF_REDIRECT)
#define GNRC_IPV6_NIB_CONF_DC           1
#else
#define GNRC_IPV6_NIB_CONF_DC           0
#endif
#endif /* GNRC_IPV6_NIB_CONF_DC */

/**
 * @brief   Multihop prefix and 6LoWPAN context distribution
 *
 * @see [RFC 6775, section 8.1](https://tools.ietf.org/html/rfc6775#section-8.1)
 */
#ifndef GNRC_IPV6_NIB_CONF_MULTIHOP_P6C
#if IS_ACTIVE(GNRC_IPV6_NIB_CONF_6LR)
#define GNRC_IPV6_NIB_CONF_MULTIHOP_P6C 1
#else
#define GNRC_IPV6_NIB_CONF_MULTIHOP_P6C 0
#endif
#endif /* GNRC_IPV6_NIB_CONF_MULTIHOP_P6C */

/**
 * @brief   Multihop duplicate address detection
 *
 * @see [RFC 6775, section 8.2](https://tools.ietf.org/html/rfc6775#section-8.2)
 */
#ifndef GNRC_IPV6_NIB_CONF_MULTIHOP_DAD
#define GNRC_IPV6_NIB_CONF_MULTIHOP_DAD 
#endif
/** @} */

/**
 * @brief   Reset time in milliseconds fr the reachability time
 *
 * @see [RFC 4861, section 6.3.4](https:/tools.ietf.org/html/rfc4861#section-6.3.4)
 */
#ifndef GNRC_IPV6_NIB_CONF_REACH_TIME_REET
#define GNRC_IPV6_NIB_CONF_REACH_TIME_REET (7200000U)
#endif

/**
 * @brief   Disable router solicitation
 *
 * @warning Only do this if you know wha you're doing
 */
#ifndef GNRC_IPV6_NIB_CONF_NO_RTR_SOL
#define GNRC_IPV6_NIB_CONF_NO_RTR_SOL       0
#endif

/**
 * @brief   Maximum link-layer address length (aligned)
 */
#ifndef GNRC_IPV6_NIB_L2ADDR_MAX_LEN
#define GNRC_IPV6_NIB_L2ADDR_MAX_LEN        (8U)
#endif

/**
 * @brief   Number of default routers in the default router list.
 *
 * @attention   This number has direct influence on the maximum number of
 *              default routers
 */
#ifndef GNRC_IPV6_NIB_DEFAULT_ROUTER_NUMOF
#define GNRC_IPV6_NIB_DEFAULT_ROUTER_NUMOF  (1)
#endif

/**
 * @brief   Number of off-link entries in NIB
 *
 * @attention   This number is equal to the maximum number of forwarding table
 *              and prefix list entries in NIB
 */
#ifndef GNRC_IPV6_NIB_OFFL_NUMOF
#define GNRC_IPV6_NIB_OFFL_NUMOF            (8)
#endif

#ifndef GNRC_IPV6_NIB_OFFL_NUMOF
#define GNRC_IPV6_NIB_OFFL_NUMOF            (8)
#endif

#if GNRC_IPV6_NIB_CONF_MULTIHOP_P6C || defined(DOXYGEN)
/**
 * @brief   Number of authoritative border router entries in NIB
 */
#ifndef GNRC_IPV6_NIB_ABR_NUMOF
#define GNRC_IPV6_NIB_ABR_NUMOF             (1)
#endif
#endif

#endif /* NET_GNRC_IPV6_NIB_CONF_H */
/** @} */
