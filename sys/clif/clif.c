/*
 * Copyright (c) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_clif
 * @{
 *
 * @file
 * @brief       CoRE Link format encoding and decoding library implementation
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "clif.h"
#include "clif_internal.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/* returns the correspondant parameter string */
static const char *_param_to_str[] = {
    LF_PARAM_ANCHOR, LF_PARAM_REL_TYPE, LF_PARAM_LANG, LF_PARAM_MEDIA,
    LF_PARAM_TITLE, LF_PARAM_TITLE_EXT, LF_PARAM_TYPE, LF_PARAM_RES_TYPE,
    LF_PARAM_IF_DESC, LF_PARAM_SIZE, LF_PARAM_CT, LF_PARAM_OBS, NULL
};

/* returns the correspondant parameter string size */
static const unsigned _param_to_size[] = {
    LF_PARAM_ANCHOR_S, LF_PARAM_REL_TYPE_S, LF_PARAM_LANG_S, LF_PARAM_MEDIA_S,
    LF_PARAM_TITLE_S, LF_PARAM_TITLE_EXT_S, LF_PARAM_TYPE_S,
    LF_PARAM_RES_TYPE_S, LF_PARAM_IF_DESC_S, LF_PARAM_SIZE_S, LF_PARAM_CT_S,
    LF_PARAM_OBS_S, 0
};

/* do not count extension param type */
#define PARAMS_NUMOF (sizeof(_param_to_str) / sizeof(_param_to_str[0]) - 1)

ssize_t clif_decode_link(clif_t *link, clif_param_t *params, unsigned params_len,
                         char *buf, size_t maxlen)
{
    assert(buf && link);
    char *pos = buf;
    char *end = buf + maxlen;

    char *link_start;
    link->params = params;
    link->target_len = 0;
    link->params_len = 0;

    ssize_t size = clif_get_link(pos, end - pos, &link_start);
    if (size < 0) {
        return CLIF_NOT_FOUND;
    }
    pos = link_start + size;

    /* now get the target and params of the link */
    size = clif_get_target(link_start, size, &link->target);
    if (size < 0) {
        return CLIF_NOT_FOUND;
    }
    link->target_len = size;
    char *pos_in_link = link->target + size;

    /* if there is no parameters array iterate all the buffer, if not until all
     * the array is used */
    while ((!params && pos_in_link < pos) || (params && link->params_len < params_len)) {
        clif_param_t *param = params ? &params[link->params_len] : NULL;
        ssize_t param_size = clif_get_param(pos_in_link, pos - pos_in_link,
                                            param);
        if (param_size < 0) {
            break;
        }
        pos_in_link += param_size;
        link->params_len++;
    }

    return pos - buf;
}

ssize_t clif_encode_link(clif_t *link, char *buf, size_t maxlen)
{
    assert(link);
    size_t pos = 0;
    ssize_t res = 0;

    res = clif_add_target(link->target, buf, maxlen);
    if (res <= 0) {
        return res;
    }
    pos += res;

    for (unsigned i = 0; i < link->params_len; i++) {
        res = clif_add_param(&link->params[i], buf ? &buf[pos] : NULL,
                             maxlen - pos);
        if (res <= 0) {
            return res;
        }
        pos += res;
    }
    return pos;
}

ssize_t clif_add_target(const char *target, char *buf, size_t maxlen)
{
    assert(target);

    size_t pos = 0;
    size_t target_len = strlen(target);

    if (!buf) {
        return target_len + 2;
    }

    if ((target_len + 2) > maxlen) {
        return CLIF_NO_SPACE;
    }

    buf[pos++] = LF_PATH_BEGIN_C;

    memcpy(&buf[pos], target, target_len);
    pos += target_len;

    buf[pos++] = LF_PATH_END_C;

    return pos;
}

ssize_t clif_add_link_separator(char *buf, size_t maxlen)
{
    if (!buf) {
        return 1;
    }

    if (maxlen < 1) {
        return CLIF_NO_SPACE;
    }

    *buf = LF_LINK_SEPARATOR_C;
    return 1;
}

ssize_t clif_add_param(clif_param_t *param, char *buf,
                              size_t maxlen)
{
    assert(param && param->key);

    param->key_len = strlen(param->key);
    /* count param name size and separator ';' */
    size_t req_space = param->key_len + 1;
    size_t pos = 0;
    int quoted = strcmp(param->key, LF_PARAM_SIZE) ? 1 : 0;

    if (param->value) {
        if (!param->value_len) {
            param->value_len = strlen(param->value);
        }
        /* count also '=' */
        req_space += param->value_len +  1;
    }

    if (quoted) {
        req_space += 2;
    }

    if (!buf) {
        return req_space;
    }

    if (req_space > maxlen) {
        return CLIF_NO_SPACE;
    }

    /* add parameter separator ';' */
    buf[pos++] = LF_PARAM_SEPARATOR_C;

    /* add parameter name */
    memcpy(&buf[pos], param->key, param->key_len);
    pos += param->key_len;

    /* add parameter value if defined */
    if (param->value) {
        buf[pos++] = LF_PARAM_ATTR_SEPARATOR_C;

        if (quoted) {
            buf[pos++] = '"';
        }

        memcpy(&buf[pos], param->value, param->value_len);
        pos += param->value_len;

        if (quoted) {
            buf[pos++] = '"';
        }
    }

    return pos;
}

ssize_t clif_get_link(const char *input, size_t input_len, char **output)
{
    assert(input);
    char *link_end;

    ssize_t path_size = clif_get_target(input, input_len, output);
    if (path_size < 0) {
        DEBUG("Path not found\n");
        return CLIF_NOT_FOUND;
    }

    (*output)--;
    /* look for link separator ',' */
    link_end = memchr(*output, LF_LINK_SEPARATOR_C,
                      (input_len - (*output - input)));

    if (!link_end) {
        /* there is no other link */
        return (input_len - (*output - input));
    }
    else {
        return (link_end - *output);
    }
}

ssize_t clif_get_target(const char *input, size_t input_len, char **output)
{
    assert(input);
    char *target_end;

    *output = memchr(input, LF_PATH_BEGIN_C, input_len);
    if (!*output) {
        DEBUG("Path start not found\n");
        return CLIF_NOT_FOUND;
    }
    *output += 1;

    target_end = memchr(*output, LF_PATH_END_C, (input + input_len) - *output);
    if (!target_end) {
        DEBUG("Path end not found\n");
        return CLIF_NOT_FOUND;
    }
    ssize_t res = target_end - *output;
    return res;
}

ssize_t clif_get_param(char *input, size_t input_len, clif_param_t *param)
{
    assert(input);
    char *param_start;
    char *input_end = input + input_len;
    char *end;
    int quoted = 0;

    if (param) {
        param->value = NULL;
        param->key = NULL;
    }

    /* look for start of parameter */
    param_start = memchr(input, LF_PARAM_SEPARATOR_C, input_len);
    if (!param_start) {
        DEBUG("Attribute separator not found\n");
        return CLIF_NOT_FOUND;
    }
    param_start++;

    /* check if there is a next parameter */
    char *next_param = memchr(param_start, LF_PARAM_SEPARATOR_C,
                              input_end - param_start);
    if (next_param) {
        input_end = next_param;
    }

    /* look for the parameter name / value separation */
    char *value = memchr(param_start, LF_PARAM_ATTR_SEPARATOR_C,
                         input_end - param_start);
    if (param) {
        param->value = value;
        param->key = param_start;
    }

    if (!value) {
        DEBUG("Attribute name / value separator not found\n");
        if (param) {
            param->key_len = input_end - param->key;
            param->value_len = 0;
        }
        return input_end - input;
    }
    else {
        if (param) {
            param->key_len = param->value - param->key;
        }
    }

    /* look for the value start */
    if (value[1] == '"') {
        quoted = true;
        value += 2;
    }
    else {
        value += 1;
    }

    if (param) {
        param->value = value;
    }

    if (quoted) {
        end = memchr(value, '"', input_end - value + 1);
        if (!end) {
            DEBUG("Closing quotes not found\n");
            return CLIF_NOT_FOUND;
        }
        else {
            if (param) {
                param->value_len = end - param->value;
            }
        }
    }
    else {
        /* if the value has no quotes, it is assumed that the rest is the
           value, though some sanity checks could be added depending on the
           parameter type */
        if (param) {
            param->value_len = input_end - param->value;
        }
    }

    return input_end - input;
}

const char *clif_param_type_to_str(clif_param_type_t type) {
    return _param_to_str[type];
}

clif_param_type_t clif_get_param_type(char *input, size_t input_len)
{
    assert(input && input_len > 0);
    clif_param_type_t ret = CLIF_PARAM_EXT;
    for (unsigned i = 0; i < PARAMS_NUMOF; i++) {
        if (input_len == _param_to_size[i] &&
            !strncmp(input, _param_to_str[i], input_len)) {
            ret = i;
            break;
        }
    }
    return ret;
}
