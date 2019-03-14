/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_link_format Link Format
 * @ingroup     sys_serialization
 * @brief       Simple encoding and decoding of Link Format strings
 *
 * @{
 *
 * @file
 * @brief       Link format encoding and decoding library public definitions
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */

#ifndef LINK_FORMAT_H
#define LINK_FORMAT_H

#include <sys/types.h>

#include "link_format_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Return types for the @ref sys_link_format API
 */
enum {
    LINK_FORMAT_OK          = 0,    /**< success */
    LINK_FORMAT_NO_SPACE    = -1,   /**< not enough space in the buffer */
    LINK_FORMAT_NOT_FOUND   = -2    /**< could not find a component in a
                                         buffer */
};

/**
 * @brief Types of link format parameters
 */
typedef enum {
    LINK_FORMAT_PARAM_ANCHOR        = 0,  /**< anchor */
    LINK_FORMAT_PARAM_REL_TYPE      = 1,  /**< rel */
    LINK_FORMAT_PARAM_LANG          = 2,  /**< hreflang */
    LINK_FORMAT_PARAM_MEDIA         = 3,  /**< media */
    LINK_FORMAT_PARAM_TITLE         = 4,  /**< title */
    LINK_FORMAT_PARAM_TITLE_EXT     = 5,  /**< title* */
    LINK_FORMAT_PARAM_TYPE          = 6,  /**< type */
    LINK_FORMAT_PARAM_RES_TYPE      = 7,  /**< rt */
    LINK_FORMAT_PARAM_IF_DESC       = 8,  /**< if */
    LINK_FORMAT_PARAM_SIZE          = 9,  /**< sz */
    LINK_FORMAT_PARAM_CT            = 10, /**< ct */
    LINK_FORMAT_PARAM_EXTENSION     = 11  /**< extensions */
} link_format_param_type_t;

/**
 * @brief Link format parameter descriptor
 */
typedef struct {
    link_format_param_type_t type; /**< type of parameter */
    char *value;                   /**< string with the value */
    unsigned value_len;            /**< length of the value */
    char *ext;                     /**< parameter name in case of extension */
    unsigned ext_len;              /**< length of the parameter name in case of
                                        extension */
} link_format_param_t;

/**
 * @brief Link format descriptor
 */
typedef struct {
    char *target;                 /**< target string */
    unsigned target_len;          /**< length of target string */
    link_format_param_t *params;  /**< array of parameters */
    unsigned params_len;          /**< size of array of parameters */
} link_format_t;

/**
 * @brief Encodes a given link in link format into a given buffer
 *
 * @param[in] link      link to encode
 * @param[out] buf      buffer to output the encoded link
 * @param[in] maxlen    size of @p buf
 *
 * @return amount of bytes used from @p buf in success
 * @return LINK_FORMAT_NO_SPACE if there is not enough space in the buffer
 */
ssize_t link_format_encode_link(link_format_t *link, char *buf, size_t maxlen);

/**
 * @brief Decodes a string of link format
 *
 * @param[out] links        array of links to populate
 * @param[in]  links_len    length of @p links
 * @param[out] params       array of params to populate
 * @param[in]  params_len   length of @p params
 * @param[in]  buf          string to decode
 * @param[in]  maxlen       size of @p buf
 *
 * @return amount of decoded links in success
 * @return LINK_FORMAT_NOT_FOUND if the string is malformed
 */
unsigned link_format_decode_links(link_format_t *links, unsigned links_len,
                                  link_format_param_t *params,
                                  unsigned params_len, char *buf, size_t maxlen);

/**
 * @brief   Adds a given @p target to a given buffer @p buf using link format
 *
 * @param[in]  target  string containing the path to the resource
 * @param[out] buf     buffer to output the formatted path
 * @param[in]  maxlen  size of @p buf
 *
 * @return in success the amount of bytes used in the buffer
 * @return LINK_FORMAT_NO_SPACE if there is not enough space in the buffer
 */
ssize_t link_format_add_target(const char *target, char *buf, size_t maxlen);

/**
 * @brief   Adds a given @p parameter to a given buffer @p buf using link format
 *
 * @param[in] param     pointer to the parameter to add
 * @param[in] buf       buffer to add the parameter to
 * @param[in] maxlen    size of @p buf
 *
 * @return amount of bytes used from the buffer if successful
 * @return LINK_FORMAT_NO_SPACE if there is not enough space in the buffer
 */
ssize_t link_format_add_param(link_format_param_t *param, char *buf,
                              size_t maxlen);

/**
 * @brief   Adds the link separator character to a given @p buf, using link
 *          format
 *
 * @param[in] buf       buffer to add the separator to
 * @param[in] maxlen    size of @p buf
 *
 * @return amount of bytes used from buffer if successful
 * @return LINK_FORMAT_NO_SPACE if there is not enough space in the buffer
 */
ssize_t link_format_add_link_separator(char *buf, size_t maxlen);

/**
 * @brief   Looks for a link in a given string @p input. If multiple link are
 *          present in the string it will return the first one.
 *
 * @param[in] input         string where to look for a link
 * @param[in] input_len     length of @p input
 * @param[out] output       if a link is found this will point to the beginning
 *                          of it
 *
 * @return length of the link if found
 * @return LINK_FORMAT_NOT_FOUND if no valid link is found
 */
ssize_t link_format_get_link(const char *input, size_t input_len,
                             char **output);

/**
 * @brief   Looks for a the target URI of a given link.
 *
 * @param[in]  input        string where to look for the target. It should only
 *                          be ONE link.
 * @param[in]  input_len    length of @p input.
 * @param[out] output       if a target is found this will point to the
 *                          beginning of it
 *
 * @return length of the target if found
 * @return LINK_FORMAT_NOT_FOUND if no valid target is found
 */
ssize_t link_format_get_target(const char *input, size_t input_len,
                               char **output);
/**
 * @brief   Looks for the first parameter in a given link.
 *
 * @param[in]  input        string where to look for the parameter. It should
 *                          only be ONE link.
 * @param[in]  input_len    length of @p input
 * @param[out] param        pointer to store the found parameter information
 *
 * @return length of the parameter in the buffer if found
 * @return LINK_FORMAT_NOT_FOUND if no valid parameter is found
 */
ssize_t link_format_get_param(char *input, size_t input_len,
                              link_format_param_t *param);

/**
 * @brief   Returns the string of a given parameter type
 *
 * @param[in] type      type of the parameter
 *
 * @return string that represents the type
 */
const char *link_format_param_type_to_str(link_format_param_type_t type);

#ifdef __cplusplus
}
#endif

#endif /* LINK_FORMAT_H */
/** @} */
