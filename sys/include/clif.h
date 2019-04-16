/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_clif CoRE Link Format
 * @ingroup     sys_serialization
 * @brief       Simple encoding and decoding of CoRE Link Format strings
 *
 * @{
 *
 * @file
 * @brief       CoRE Link Format encoding and decoding library public
 *              definitions
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */

#ifndef CLIF_H
#define CLIF_H

#include <sys/types.h>

#include "clif_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Return types for the @ref sys_clif API
 */
enum {
    CLIF_OK          = 0,    /**< success */
    CLIF_NO_SPACE    = -1,   /**< not enough space in the buffer */
    CLIF_NOT_FOUND   = -2    /**< could not find a component in a buffer */
};

/**
 * @brief Types of link format parameters
 */
typedef enum {
    CLIF_PARAM_ANCHOR        = 0,  /**< anchor */
    CLIF_PARAM_REL           = 1,  /**< rel */
    CLIF_PARAM_LANG          = 2,  /**< hreflang */
    CLIF_PARAM_MEDIA         = 3,  /**< media */
    CLIF_PARAM_TITLE         = 4,  /**< title */
    CLIF_PARAM_TITLE_EXT     = 5,  /**< title* */
    CLIF_PARAM_TYPE          = 6,  /**< type */
    CLIF_PARAM_RT            = 7,  /**< rt */
    CLIF_PARAM_IF            = 8,  /**< if */
    CLIF_PARAM_SZ            = 9,  /**< sz */
    CLIF_PARAM_CT            = 10, /**< ct */
    CLIF_PARAM_OBS           = 11, /**< obs */
    CLIF_PARAM_EXT           = 12  /**< extensions */
} clif_param_type_t;

/**
 * @brief Link format parameter descriptor
 */
typedef struct {
    char *value;                   /**< string with the value */
    unsigned value_len;            /**< length of the value */
    char *key;                     /**< parameter name */
    unsigned key_len;              /**< length of the parameter name */
} clif_param_t;

/**
 * @brief Link format descriptor
 */
typedef struct {
    char *target;                 /**< target string */
    unsigned target_len;          /**< length of target string */
    clif_param_t *params;         /**< array of parameters */
    unsigned params_len;          /**< size of array of parameters */
} clif_t;

/**
 * @brief Encodes a given link in link format into a given buffer
 *
 * @param[in] link      link to encode
 * @param[out] buf      buffer to output the encoded link. Can be NULL
 * @param[in] maxlen    size of @p buf
 *
 * @note If @p buf is NULL this will return the amount of bytes that would be
 *       needed
 *
 * @return amount of bytes used from @p buf in success
 * @return CLIF_NO_SPACE if there is not enough space in the buffer
 */
ssize_t clif_encode_link(clif_t *link, char *buf, size_t maxlen);

/**
 * @brief   Decodes a string of link format. It decodes the first occurrence of
 *          a link.
 *
 * @param[out] link         link to populate
 * @param[in]  params       array of params to populate
 * @param[in]  params_len   length of @p params
 * @param[in]  buf          string to decode
 * @param[in]  maxlen       size of @p buf
 *
 * @return number of bytes parsed from @p buf in success
 * @return CLIF_NOT_FOUND if the string is malformed
 */
ssize_t clif_decode_link(clif_t *link, clif_param_t *params, unsigned params_len,
                         char *buf, size_t maxlen);

/**
 * @brief   Adds a given @p target to a given buffer @p buf using link format
 *
 * @param[in]  target  string containing the path to the resource
 * @param[out] buf     buffer to output the formatted path. Can be NULL
 * @param[in]  maxlen  size of @p buf
 *
 * @note If @p buf is NULL this will return the amount of bytes that would be
 *       needed
 *
 * @return in success the amount of bytes used in the buffer
 * @return CLIF_NO_SPACE if there is not enough space in the buffer
 */
ssize_t clif_add_target(const char *target, char *buf, size_t maxlen);

/**
 * @brief   Adds a given @p parameter to a given buffer @p buf using link format
 *
 * @param[in] param     pointer to the parameter to add
 * @param[in] buf       buffer to add the parameter to. Can be NULL
 * @param[in] maxlen    size of @p buf
 *
 * @note If @p buf is NULL this will return the amount of bytes that would be
 *       needed
 *
 * @return amount of bytes used from the buffer if successful
 * @return CLIF_NO_SPACE if there is not enough space in the buffer
 */
ssize_t clif_add_param(clif_param_t *param, char *buf,
                              size_t maxlen);

/**
 * @brief   Adds the link separator character to a given @p buf, using link
 *          format
 *
 * @param[in] buf       buffer to add the separator to. Can be NULL
 * @param[in] maxlen    size of @p buf
 *
 * @note If @p buf is NULL this will return the amount of bytes that would be
 *       needed
 *
 * @return amount of bytes used from buffer if successful
 * @return CLIF_NO_SPACE if there is not enough space in the buffer
 */
ssize_t clif_add_link_separator(char *buf, size_t maxlen);

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
 * @return CLIF_NOT_FOUND if no valid link is found
 */
ssize_t clif_get_link(const char *input, size_t input_len,
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
 * @return CLIF_NOT_FOUND if no valid target is found
 */
ssize_t clif_get_target(const char *input, size_t input_len,
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
 * @return CLIF_NOT_FOUND if no valid parameter is found
 */
ssize_t clif_get_param(char *input, size_t input_len,
                              clif_param_t *param);

/**
 * @brief   Returns the parameter type of a given string.
 *
 * @param[in] input             string containing the parameter name
 * @param[in] input_len         length of @p input
 *
 * @return  type of the parameter
 */
clif_param_type_t clif_get_param_type(char *input, size_t input_len);

/**
 * @brief   Returns the string of a given parameter type
 *
 * @param[in] type      type of the parameter
 *
 * @return string that represents the type
 */
const char *clif_param_type_to_str(clif_param_type_t type);

#ifdef __cplusplus
}
#endif

#endif /* CLIF_H */
/** @} */
