/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup sys_coral
 * @{
 * @file
 * @brief   Functions to encode and decode CoRE CoRAL
 *
 * @author  Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 *
 */

#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "cn-cbor/cn-cbor.h"
#include "memarray.h"
#include "coral.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#define CORAL_CBOR_NUM_RECORDS  (128)


typedef struct {
    coral_element_t *pool;
    unsigned pool_len;
    unsigned cur;
} _coral_element_pool_t;

typedef struct {
    _coral_element_pool_t *pool;
    coral_element_t *parent;
    coral_element_t *current;
} _decode_ctx_t;

/* CoRAL document traversing */
typedef void (*coral_visitor_t)(coral_element_t *e, int depth, void *context);
static void _visit(coral_element_t *cb, coral_visitor_t visitor, void *context);
static void _print_visited(coral_element_t *e, int depth, void *context);

/* encoding functions */
static void _encode_visited(coral_element_t *e, int depth, void *context);
static void _encode_link(coral_element_t *e);
static void _encode_form(coral_element_t *e);
static void _encode_rep(coral_element_t *e);

/* decoding functions */
static int _decode_cbor(cn_cbor *cb, _coral_element_pool_t *pool);

/* coral element pool */
static coral_element_t *_get_from_element_pool(_coral_element_pool_t *pool);
static void _init_element(coral_element_t *e);

/* CBOR memory allocation */
static void *_cbor_calloc(size_t count, size_t size, void *memblock);
static void _cbor_free(void *ptr, void *memblock);

static cn_cbor _block_storage_data[CORAL_CBOR_NUM_RECORDS];
static memarray_t _storage;
static cn_cbor_context _ct = {
    .calloc_func = _cbor_calloc,
    .free_func = _cbor_free,
    .context = &_storage
};

void coral_create_document(coral_element_t *root)
{
    root->type = CORAL_TYPE_BODY;
    root->next = NULL;
    root->children = NULL;
    root->children_n = 0;
    root->parent = NULL;
}

void coral_create_link(coral_element_t *link, char *rel, coral_link_target_t *target)
{
    link->type = CORAL_TYPE_LINK;
    link->next = NULL;
    link->children = NULL;
    link->children_n = 0;
    link->parent = NULL;
    link->v.link.rel_type.str = rel;
    link->v.link.rel_type.len = strlen(rel);
    memcpy(&link->v.link.target, target, sizeof(coral_link_target_t));
}

void coral_create_form(coral_element_t *form, char *op, uint8_t method,
                       char *target)
{
    form->type = CORAL_TYPE_FORM;
    form->next = NULL;
    form->children = NULL;
    form->children_n = 0;
    form->parent = NULL;
    form->v.form.method = method;
    form->v.form.op_type.str = op;
    form->v.form.op_type.len = strlen(op);
    form->v.form.target.str = target;
    form->v.form.target.len = strlen(target);
}

void coral_create_rep(coral_element_t *rep, uint8_t *buf, size_t buf_len)
{
    rep->type = CORAL_TYPE_REP;
    rep->next = NULL;
    rep->children = NULL;
    rep->children_n = 0;
    rep->parent = NULL;
    rep->v.rep.bytes = buf;
    rep->v.rep.bytes_len = buf_len;
}

int coral_append_element(coral_element_t *root, coral_element_t *el)
{
    root->children_n++;
    el->next = NULL;
    el->parent = root;

    if (!root->children) {
        root->children = el;
    }
    else {
        root->last_child->next = el;
    }

    root->last_child = el;
    return 0;
}

int coral_decode(coral_element_t *e, unsigned e_len, uint8_t *buf,
                 size_t buf_len)
{
    _coral_element_pool_t pool = { .pool = e, .pool_len = e_len, .cur = 0 };
    cn_cbor_errback err;

    cn_cbor *cbor_root = cn_cbor_decode(buf, buf_len, &_ct, &err);
    if (!cbor_root) {
        DEBUG("CBOR root empty\n");
        DEBUG("Error code: %d at pos %d\n", err.err, err.pos);
    }
    (void)cbor_root;
    (void)e;
    (void)e_len;
    _decode_cbor(cbor_root, &pool);
    return -1;
}

ssize_t coral_encode(coral_element_t *root, uint8_t *buf, size_t buf_len)
{
    (void)root;
    ssize_t pos = 0;
    memarray_init(&_storage, _block_storage_data, sizeof(cn_cbor),
                  CORAL_CBOR_NUM_RECORDS);
    
    _visit(root, _encode_visited, NULL);
    pos += cn_cbor_encoder_write(buf, pos, buf_len, root->cbor_body);
    cn_cbor_free(root->cbor_body, &_ct);
    return pos;
}

void coral_print_structure(coral_element_t *root)
{
    _visit(root, _print_visited, NULL);
}

static void _encode_visited(coral_element_t *e, int depth, void *context)
{
    (void)depth;
    (void)context;

    if (e->type != CORAL_TYPE_BODY) {
        e->cbor_root = cn_cbor_array_create(&_ct, NULL);
    }

    switch (e->type) {
        case CORAL_TYPE_LINK:
            _encode_link(e);
            break;
        case CORAL_TYPE_FORM:
            _encode_form(e);
            break;
        case CORAL_TYPE_REP:
            _encode_rep(e);
            break;
        case CORAL_TYPE_BODY:
            break;
        default:
            DEBUG("Unknown coral element\n");
    }

    if (e->children) {
        e->cbor_body = cn_cbor_array_create(&_ct, NULL);
        cn_cbor_array_append(e->cbor_root, e->cbor_body, NULL);
    }

    if (e->parent && e->parent->cbor_body) {
        cn_cbor_array_append(e->parent->cbor_body, e->cbor_root, NULL);
    }
}

static void _encode_link(coral_element_t *e)
{
    cn_cbor *val = cn_cbor_int_create(CORAL_TYPE_LINK, &_ct, NULL);
    cn_cbor_array_append(e->cbor_root, val, NULL);

    val = cn_cbor_string_create(e->v.link.rel_type.str, &_ct, NULL);
    cn_cbor_array_append(e->cbor_root, val, NULL);

    switch (e->v.link.target.type) {
        case CORAL_LITERAL_TEXT:
            val = cn_cbor_string_create(e->v.link.target.v.as_str.str, &_ct, NULL);
            break;
        case CORAL_LITERAL_BOOL:
            val = cn_cbor_int_create(0, &_ct, NULL);
            val->type = e->v.link.target.v.as_int ? CN_CBOR_TRUE : CN_CBOR_FALSE;
            break;
        case CORAL_LITERAL_BYTES:
            val = cn_cbor_data_create(e->v.link.target.v.as_bytes.bytes, e->v.link.target.v.as_bytes.bytes_len, &_ct, NULL);
            break;
        case CORAL_LITERAL_INT:
            val = cn_cbor_int_create(e->v.link.target.v.as_int, &_ct, NULL);
            break;
        case CORAL_LITERAL_FLOAT:
            val = cn_cbor_float_create(e->v.link.target.v.as_float, &_ct, NULL);
            break;
        default:
            DEBUG("Invalid literal type");
            return;
    }
    cn_cbor_array_append(e->cbor_root, val, NULL);
}

static void _encode_form(coral_element_t *e)
{
    cn_cbor *val = cn_cbor_int_create(CORAL_TYPE_FORM, &_ct, NULL);
    cn_cbor_array_append(e->cbor_root, val, NULL);

    val = cn_cbor_string_create(e->v.form.op_type.str, &_ct, NULL);
    cn_cbor_array_append(e->cbor_root, val, NULL);

    val = cn_cbor_int_create(e->v.form.method, &_ct, NULL);
    cn_cbor_array_append(e->cbor_root, val, NULL);

    val = cn_cbor_string_create(e->v.form.target.str, &_ct, NULL);
    cn_cbor_array_append(e->cbor_root, val, NULL);
}

static void _encode_rep(coral_element_t *e)
{
    cn_cbor *val = cn_cbor_int_create(CORAL_TYPE_REP, &_ct, NULL);
    cn_cbor_array_append(e->cbor_root, val, NULL);

    val = cn_cbor_data_create(e->v.rep.bytes, e->v.rep.bytes_len, &_ct,
                              NULL);
    cn_cbor_array_append(e->cbor_root, val, NULL);
}


static void _print_visited(coral_element_t *e, int depth, void *context)
{
    (void)context;
    DEBUG("|--");
    for (int i = 0; i < depth; i++) {
        DEBUG("|--");
    }
    DEBUG("|(%d children)", e->children_n);
    DEBUG(" type: ");

    switch (e->type) {
        case CORAL_TYPE_LINK:
            DEBUG("link - rel: %.*s <%.*s>", e->v.link.rel_type.len,
                   e->v.link.rel_type.str, e->v.link.target.v.as_str.len,
                   e->v.link.target.v.as_str.str);
            break;

        case CORAL_TYPE_FORM:
            DEBUG("form - op: %.*s", e->v.form.op_type.len,
                   e->v.form.op_type.str);
            break;

        case CORAL_TYPE_BODY:
            DEBUG("document body");
            break;
        case CORAL_TYPE_REP:
            DEBUG("embedded representation: ");
            for (unsigned i = 0; i < e->v.rep.bytes_len; i++) {
                DEBUG("%#x ", e->v.rep.bytes[i]);
            }
            break;
        default:
            DEBUG("undefined");
            break;
    }
    DEBUG("\n");
}

static void _visit(coral_element_t *e, coral_visitor_t visitor, void *context)
{
    coral_element_t *p = e;
    int depth = 0;
    while (p) {
visit:
      visitor(p, depth, context);
      if (p->children) {
        //DEBUG("visiting children\n");
        p = p->children;
        depth++;
      } else{
        if (p->next) {
          //DEBUG("visiting sibling\n");
          p = p->next;
        } else {
          while (p->parent) {
            //DEBUG("back to parent\n");
            depth--;
            if (p->parent->next) {
              //DEBUG("moving to uncle\n");
              p = p->parent->next;
              goto visit;
            }
            //DEBUG("moving to grandparent\n");
            p = p->parent;
          }
          return;
        }
      }
    }
}

static int _decode_link(cn_cbor *cb, cn_cbor **body, _decode_ctx_t *ctx)
{
    cn_cbor *p;
    ctx->current = _get_from_element_pool(ctx->pool);
    ctx->current->parent = ctx->parent;
    ctx->current->type = CORAL_TYPE_LINK;
    ctx->current->next = NULL;
    ctx->current->children_n = 0;
    ctx->current->children = NULL;
    ctx->current->last_child = NULL;

    /* rel type */
    p = cb->first_child->next;
    if (!p || p->type != CN_CBOR_TEXT) {
        DEBUG("Error, link rel type should be text. Type is: %d\n", p->type);
        return -1;
    }
    ctx->current->v.link.rel_type.str = p->v.str;
    ctx->current->v.link.rel_type.len = p->length;

    /* target */
    p = p->next;
    if (!p || p->type != CN_CBOR_TEXT) { // TODO: Should accept CIRI and literal
        DEBUG("Error, link target only can be text for now\n");
        return -1;
    }
    ctx->current->v.link.target.v.as_str.str = p->v.str;
    ctx->current->v.link.target.v.as_str.len = p->length;

    DEBUG("We found a link:\n");
    DEBUG("- Rel: %.*s\n", ctx->current->v.link.rel_type.len, ctx->current->v.link.rel_type.str);
    DEBUG("- Target: %.*s\n", ctx->current->v.link.target.v.as_str.len, ctx->current->v.link.target.v.as_str.str);

    coral_append_element(ctx->parent, ctx->current);

    /* if has a body point to it */
    if (p->next && p->next->type == CN_CBOR_ARRAY) {
        DEBUG("Link has a body\n");
        *body = p->next->first_child;
    }
    DEBUG("\n");
    return 0;
}

static int _decode_form(cn_cbor *cb, cn_cbor **body, _decode_ctx_t *ctx)
{
    cn_cbor *p;
    ctx->current = _get_from_element_pool(ctx->pool);
    ctx->current->parent = ctx->parent;
    ctx->current->type = CORAL_TYPE_FORM;
    ctx->current->next = NULL;
    ctx->current->children_n = 0;
    ctx->current->children = NULL;
    ctx->current->last_child = NULL;

    /* op type */
    p = cb->first_child->next;
    if (!p || p->type != CN_CBOR_TEXT) {
        DEBUG("Error, form op type should be text\n");
        return -1;
    }
    ctx->current->v.form.op_type.str = p->v.str;
    ctx->current->v.form.op_type.len = p->length;

    /* method */
    p = p->next;
    if (!p || p->type != CN_CBOR_UINT) {
        DEBUG("Error, form method can only be integer\n");
        return -1;
    }
    ctx->current->v.form.method = p->v.uint;

    /* target */
    p = p->next;
    if (!p || p->type != CN_CBOR_TEXT) { // TODO: Should accept CIRI and literal
        DEBUG("Error, form target only can be text for now\n");
        return -1;
    }
    ctx->current->v.form.target.str = p->v.str;
    ctx->current->v.form.target.len = p->length;

    DEBUG("We found a form:\n");
    DEBUG("- Op type: %.*s\n", ctx->current->v.form.op_type.len, ctx->current->v.form.op_type.str);
    DEBUG("- Method: %d\n", ctx->current->v.form.method);
    DEBUG("- Target: %.*s\n", ctx->current->v.form.target.len, ctx->current->v.form.target.str);

    coral_append_element(ctx->parent, ctx->current);

    /* if has a body point to it */
    if (p->next && p->next->type == CN_CBOR_ARRAY) {
        DEBUG("Form has a body\n");
        *body = p->next->first_child;
    }
    DEBUG("\n");
    return 0;
}

static int _decode_rep(cn_cbor *cb, cn_cbor **body, _decode_ctx_t *ctx)
{
    cn_cbor *p;
    ctx->current = _get_from_element_pool(ctx->pool);
    ctx->current->type = CORAL_TYPE_REP;

    /* bytes */
    p = cb->first_child->next;
    if (!p || p->type != CN_CBOR_BYTES) {
        DEBUG("Error, embedded representation bytes should be bytes. Type is: %d\n", p->type);
        return -1;
    }
    ctx->current->v.rep.bytes = p->v.bytes;
    ctx->current->v.rep.bytes_len = p->length;

    DEBUG("We found an embedded representation\n");
    DEBUG("- Amount of bytes: %d\n", ctx->current->v.rep.bytes_len);

    coral_append_element(ctx->parent, ctx->current);

    /* if has a body point to it */
    if (p->next && p->next->type == CN_CBOR_ARRAY) {
        DEBUG("Rep has a body\n");
        *body = p->next->first_child;
    }
    DEBUG("\n");
    return 0;
}

static int _decode_cbor(cn_cbor *cb, _coral_element_pool_t *pool)
{
    _decode_ctx_t ctx;
    cn_cbor *p = cb->first_child;
    ctx.pool = pool;
    ctx.parent = _get_from_element_pool(pool);
    coral_create_document(ctx.parent);
    ctx.current = NULL;

    while (p) {
    cn_cbor *res = NULL;
element:
        if (p->type != CN_CBOR_ARRAY || !p->first_child) {
            DEBUG("Error, should be a CBOR array\n");
            return -1;
        }
        if (p->first_child->type != CN_CBOR_UINT) {
            DEBUG("Error, all elements should start with code. Type is %d\n", p->first_child->type);
            return -1;
        }

        switch(p->first_child->v.uint) {
            case CORAL_TYPE_LINK:
                if (_decode_link(p, &res, &ctx) < 0) {
                    DEBUG("error parsing link\n");
                    return -1;
                }
                break;
            case CORAL_TYPE_FORM:
                if (_decode_form(p, &res, &ctx) < 0) {
                    DEBUG("error parsing form\n");
                    return -1;
                }
                break;
            case CORAL_TYPE_REP:
                if (_decode_rep(p, &res, &ctx) < 0) {
                    DEBUG("error parsing rep\n");
                    return -1;
                }
                break;
            default:
                DEBUG("unknown element\n");
                break;
        }
        if (res) {
            /* element has children */
            ctx.parent = ctx.current;
            p = res;
        }
        else {
            if (p->next) {
                DEBUG("Moving to next element\n");
                p = p->next;
            }
            else {
                DEBUG("There is no next element\n");
                while (p->parent) {
                    if (p->parent->next) {
                        ctx.parent = ctx.parent->parent;
                        p = p->parent->next;
                        goto element;
                    }
                    p = p->parent;
                }
            }
        }
    }
    return 0;
}

static coral_element_t *_get_from_element_pool(_coral_element_pool_t *pool)
{
    if (pool->cur < pool->pool_len) {
        coral_element_t *ret = &pool->pool[pool->cur++];
        ret->type = CORAL_TYPE_UNDEF;
        _init_element(ret);
        return ret;
    }
    else {
        return NULL;
    }
}

static void _init_element(coral_element_t *e)
{
    e->type = CORAL_TYPE_UNDEF;
    e->parent = NULL;
    e->children_n = 0;
    e->children = NULL;
    e->next = NULL;
    e->last_child = NULL;
    e->cbor_body = NULL;
    e->cbor_root = NULL;
}

static void *_cbor_calloc(size_t count, size_t size, void *memblock)
{
    (void)count;
    void *block = memarray_alloc(memblock);
    if (block) {
        memset(block, 0, size);
    }
    return block;
}

static void _cbor_free(void *ptr, void *memblock)
{
    memarray_free(memblock, ptr);
}

