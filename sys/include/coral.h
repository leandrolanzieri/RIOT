/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_coral CoRAL encoder decoder
 * @ingroup     sys_serialization
 * @brief       CoRE CoRAL encoder and decoder
 * @{
 *
 * @brief       encoding and decoding functions for CoRE CoRAL
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */

#ifndef CORAL_H
#define CORAL_H

#include <stddef.h> /* for size_t */
#include <stdint.h>
#include "sys/types.h"

#include "cn-cbor/cn-cbor.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    CORAL_TYPE_UNDEF = -2,
    CORAL_TYPE_BODY = -1,
    CORAL_TYPE_REP = 0,
    CORAL_TYPE_DIR_BASE = 1,
    CORAL_TYPE_LINK = 2,
    CORAL_TYPE_FORM = 3
} coral_element_type_t;

typedef struct {
    const char *str;
    size_t len;
} coral_str_t;

// TODO this could be a CIRI
typedef coral_str_t coral_link_target_t;
typedef coral_str_t coral_form_target_t;

typedef struct {
    coral_str_t rel_type;
    coral_link_target_t target;
} coral_link_t;

typedef struct {
    coral_str_t op_type;
    uint8_t method;
    coral_form_target_t target;
} coral_form_t;

typedef struct {
    const uint8_t *bytes;
    size_t bytes_len;
    // TODO add metadata
} coral_rep_t;

typedef struct coral_element {
    struct coral_element *next;
    struct coral_element *children;
    struct coral_element *last_child;
    struct coral_element *parent;
    coral_element_type_t type;
    cn_cbor *cbor_root;
    cn_cbor *cbor_body;
    unsigned children_n;     // could be used to estimate memory
    union {
        coral_link_t link;
        coral_form_t form;
        coral_rep_t rep;
    } v;
} coral_element_t;

// TODO remove
void coral_print_structure(coral_element_t *root);

/**
 * @name CoRAL encoder functions
 * @{
 */
/**
 * @brief   Creates a new CoRAL document starting in @p root
 *
 * @param[out] root  element to be the root of the document
 */
void coral_create_document(coral_element_t *root);

void coral_create_link(coral_element_t *link, char *rel, char *target);

void coral_create_form(coral_element_t *form, char *op, uint8_t method,
                       char *target);

void coral_create_rep(coral_element_t *rep, uint8_t *buf, size_t buf_len);

int coral_append_element(coral_element_t *root, coral_element_t *el);

ssize_t coral_encode(coral_element_t *root, uint8_t *buf, size_t buf_len);

int coral_decode(coral_element_t *e, unsigned e_len, uint8_t *buf, size_t buf_len);

/** @} */

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* CORAL_H */
