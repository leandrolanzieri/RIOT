/*
 * Copyright (c) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_link_format
 * @{
 *
 * @file
 * @brief       Link format encoding and decoding library implementation
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "link_format.h"
#include "link_format_internal.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/* returns the correspondant parameter string */
static const char *_param_to_str[] = {
    LF_PARAM_ANCHOR, LF_PARAM_REL_TYPE, LF_PARAM_LANG, LF_PARAM_MEDIA,
    LF_PARAM_TITLE, LF_PARAM_TITLE_EXT, LF_PARAM_TYPE, LF_PARAM_RES_TYPE,
    LF_PARAM_IF_DESC, LF_PARAM_SIZE, LF_PARAM_CT, NULL
};

/* returns the correspondant parameter string size */
static const unsigned _param_to_size[] = {
    LF_PARAM_ANCHOR_S, LF_PARAM_REL_TYPE_S, LF_PARAM_LANG_S, LF_PARAM_MEDIA_S,
    LF_PARAM_TITLE_S, LF_PARAM_TITLE_EXT_S, LF_PARAM_TYPE_S,
    LF_PARAM_RES_TYPE_S, LF_PARAM_IF_DESC_S, LF_PARAM_SIZE_S, LF_PARAM_CT_S, 0
};

/* do not count extension param type */
#define PARAMS_NUMOF (sizeof(_param_to_str) / sizeof(_param_to_str[0]) - 1)

unsigned link_format_decode_links(link_format_t *links, unsigned links_len,
                                 link_format_param_t *params,
                                 unsigned params_len, char *buf, size_t maxlen)
{
    assert(buf);
    char *pos = buf;
    char *end = buf + maxlen;
    unsigned links_numof = 0;
    unsigned params_numof = 0;

    while (links_numof < links_len) {
        char *link;
        links[links_numof].target_len = 0;
        links[links_numof].params = &params[params_numof];
        links[links_numof].params_len = 0;

        /* first determine where the next link is and how long it is */
        ssize_t size = link_format_get_link(pos, end - pos, &link);
        if (size < 0) {
            goto out;
        }
        pos = link + size;

        /* now get the target and params of the link */
        size = link_format_get_target(link, size, &links[links_numof].target);
        if (size < 0) {
            return LINK_FORMAT_NOT_FOUND;
        }
        char *pos_in_link = links[links_numof].target + size;
        while (params_numof < params_len) {
            ssize_t param_size = link_format_get_param(pos_in_link,
                                                       pos - pos_in_link,
                                                       &params[params_numof]);
            if (param_size < 0) {
                break;
            }
            pos_in_link += param_size;
            links[links_numof].params_len++;
            params_numof++;
        }

        links[links_numof].target_len = size;
        links_numof++;
    }
out:
    return links_numof;
}

ssize_t link_format_encode_link(link_format_t *link, char *buf, size_t maxlen)
{
    assert(link);
    size_t pos = 0;
    ssize_t res = 0;

    res = link_format_add_target(link->target, buf, maxlen);
    if (res <= 0) {
        return res;
    }
    pos += res;

    for (unsigned i = 0; i < link->params_len; i++) {
        res = link_format_add_param(&link->params[i], &buf[pos], maxlen - pos);
        if (res <= 0) {
            return res;
        }
        pos += res;
    }
    return pos;
}

ssize_t link_format_add_target(const char *target, char *buf, size_t maxlen)
{
    assert(target);

    size_t pos = 0;
    size_t target_len = strlen(target);

    if (!buf) {
        return target_len + 2;
    }

    if ((target_len + 2) > maxlen) {
        return LINK_FORMAT_NO_SPACE;
    }

    buf[pos++] = LF_PATH_BEGIN_C;

    memcpy(&buf[pos], target, target_len);
    pos += target_len;

    buf[pos++] = LF_PATH_END_C;

    return pos;
}

ssize_t link_format_add_link_separator(char *buf, size_t maxlen)
{
    if (!buf) {
        return 1;
    }

    if (maxlen < 1) {
        return LINK_FORMAT_NO_SPACE;
    }

    *buf = LF_LINK_SEPARATOR_C;
    return 1;
}

ssize_t link_format_add_param(link_format_param_t *param, char *buf,
                              size_t maxlen)
{
    assert(param);
    /* if it is an extension the 'ext' field should be provided */
    assert(!(param->type == LINK_FORMAT_PARAM_EXTENSION && !param->ext));

    /* count param name size and separator ';' */
    size_t req_space = _param_to_size[param->type] + 1;
    size_t pos = 0;
    int quoted = (param->type == LINK_FORMAT_PARAM_SIZE) ? 0 : 1;

    if (param->value) {
        if (!param->value_len) {
            param->value_len = strlen(param->value);
        }
        /* count also '=' */
        req_space += param->value_len +  1;
    }

    if (param->type == LINK_FORMAT_PARAM_EXTENSION && !param->ext_len) {
        param->ext_len = strlen(param->ext);
        req_space += param->ext_len;
    }

    if (quoted) {
        req_space += 2;
    }

    if (!buf) {
        return req_space;
    }

    if (req_space > maxlen) {
        return LINK_FORMAT_NO_SPACE;
    }

    /* add parameter separator ';' */
    buf[pos++] = LF_PARAM_SEPARATOR_C;

    /* add parameter name */
    if (param->type == LINK_FORMAT_PARAM_EXTENSION) {
        memcpy(&buf[pos], param->ext, param->ext_len);
        pos += param->ext_len;
    }
    else {
        strcpy(&buf[pos], _param_to_str[param->type]);
        pos += _param_to_size[param->type];
    }

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

ssize_t link_format_get_link(const char *input, size_t input_len, char **output)
{
    assert(input);
    char *link_end;

    ssize_t path_size = link_format_get_target(input, input_len, output);
    if (path_size < 0) {
        DEBUG("Path not found\n");
        return LINK_FORMAT_NOT_FOUND;
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

ssize_t link_format_get_target(const char *input, size_t input_len,
                               char **output)
{
    assert(input);
    char *target_end;

    *output = memchr(input, LF_PATH_BEGIN_C, input_len);
    if (!*output) {
        DEBUG("Path start not found\n");
        return LINK_FORMAT_NOT_FOUND;
    }
    *output += 1;

    target_end = memchr(*output, LF_PATH_END_C, (input + input_len) - *output);
    if (!target_end) {
        DEBUG("Path end not found\n");
        return LINK_FORMAT_NOT_FOUND;
    }
    ssize_t res = target_end - *output;
    return res;
}

ssize_t link_format_get_param(char *input, size_t input_len,
                              link_format_param_t *param)
{
    assert(input && param);
    char *param_start;
    char *input_end = input + input_len;
    char *end;
    int quoted = 0;

    param->value = NULL;
    param->ext = NULL;

    /* look for start of parameter */
    param_start = memchr(input, LF_PARAM_SEPARATOR_C, input_len);
    if (!param_start) {
        DEBUG("Attribute separator not found\n");
        return LINK_FORMAT_NOT_FOUND;
    }
    param_start++;

    /* check if there is a next parameter */
    char *next_param = memchr(param_start, LF_PARAM_SEPARATOR_C,
                              input_end - param_start);
    if (next_param) {
        input_end = next_param;
    }

    /* look for the parameter name / value separation */
    param->value = memchr(param_start, LF_PARAM_ATTR_SEPARATOR_C,
                         input_end - param_start);
    if (!param->value) {
        DEBUG("Attribute name / value separator not found: assuming extension\n");
        /* if there is no name / value separator it is considered an extension */
        param->type = LINK_FORMAT_PARAM_EXTENSION;
        param->ext = param_start;
        param->ext_len = input_end - param->ext;
        param->value_len = 0;
        return input_end - input;
    }

    /* look for the value start */
    if ((param->value)[1] == '"') {
        quoted = true;
        param->value += 2;
    }
    else {
        param->value += 1;
    }

    if (quoted) {
        end = memchr(param->value, '"', input_end - param->value + 1);
        if (!end) {
            DEBUG("Closing quotes not found\n");
            return LINK_FORMAT_NOT_FOUND;
        }
        else {
            param->value_len = end - param->value;
        }
    }
    else {
        /* if the value has no quotes, it is assumed that the rest is the
           value, though some sanity checks could be added depending on the
           parameter type */
        param->value_len = input_end - param->value;
    }

    /* calculate the parameter name length */
    size_t param_length;
    if (param->value) {
        param_length = param->value - 2 - param_start;
    }
    else {
        param_length = input_end - param_start;
    }

    /* try to determine the parameter type */
    param->type = LINK_FORMAT_PARAM_EXTENSION;
    for (unsigned i = 0; i < PARAMS_NUMOF; i++) {
        if (!strncmp(param_start, _param_to_str[i], param_length)) {
            param->type = i;
            break;
        }
    }

    /* if it is an extension add the extra information */
    if (param->type == LINK_FORMAT_PARAM_EXTENSION) {
        param->ext = param_start;
        param->ext_len = param_length;
    }

    return input_end - input;
}

const char *link_format_param_type_to_str(link_format_param_type_t type) {
    return _param_to_str[type];
}
