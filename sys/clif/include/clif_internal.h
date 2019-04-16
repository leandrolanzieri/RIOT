/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup    sys_clif
 *
 * @{
 *
 * @file
 * @brief     Internal definitions for CoRE Link format module
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */
#ifndef CLIF_INTERNAL_H
#define CLIF_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief link format path initial character
 */
#define LF_PATH_BEGIN_C     '<'

/**
 * @brief link format path final character
 */
#define LF_PATH_END_C       '>'

/**
 * @brief link format link separator character
 */
#define LF_LINK_SEPARATOR_C ','

/**
 * @brief link format parameter separator character
 *
 */
#define LF_PARAM_SEPARATOR_C ';'

/**
 * @brief link format attribute separator character
 */
#define LF_PARAM_ATTR_SEPARATOR_C '='

/**
 * @{
 * @brief link format anchor parameter string and size
 */
#define LF_PARAM_ANCHOR      "anchor"
#define LF_PARAM_ANCHOR_S    (6)
/** @} */

/**
 * @{
 * @brief link format relation type parameter string and size
 */
#define LF_PARAM_REL_TYPE    "rel"
#define LF_PARAM_REL_TYPE_S  (3)
/** @} */

/**
 * @{
 * @brief link format language parameter string and size
 */
#define LF_PARAM_LANG        "hreflang"
#define LF_PARAM_LANG_S      (8)
/** @} */

/**
 * @{
 * @brief link format media parameter string and size
 */
#define LF_PARAM_MEDIA       "media"
#define LF_PARAM_MEDIA_S     (5)
/** @} */

/**
 * @{
 * @brief link format title parameter string and size
 */
#define LF_PARAM_TITLE       "title"
#define LF_PARAM_TITLE_S     (5)
/** @} */

/**
 * @{
 * @brief link format title extended parameter string and size
 */
#define LF_PARAM_TITLE_EXT   "title*"
#define LF_PARAM_TITLE_EXT_S (6)
/** @} */

/**
 * @{
 * @brief link format type parameter string and size
 */
#define LF_PARAM_TYPE        "type"
#define LF_PARAM_TYPE_S      (4)
/** @} */

/**
 * @{
 * @brief link format resource type parameter string and size
 */
#define LF_PARAM_RES_TYPE    "rt"
#define LF_PARAM_RES_TYPE_S  (2)
/** @} */

/**
 * @{
 * @brief link format interface description parameter string and size
 */
#define LF_PARAM_IF_DESC     "if"
#define LF_PARAM_IF_DESC_S   (2)
/** @} */

/**
 * @{
 * @brief link format size parameter string and size
 */
#define LF_PARAM_SIZE        "sz"
#define LF_PARAM_SIZE_S      (2)
/** @} */

/**
 * @{
 * @brief link format content-format string and size
 */
#define LF_PARAM_CT          "ct"
#define LF_PARAM_CT_S        (2)
/** @} */

/**
 * @{
 * @brief link format observable string and size
 */
#define LF_PARAM_OBS         "obs"
#define LF_PARAM_OBS_S       (3)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* CLIF_INTERNAL_H */
/** @} */
