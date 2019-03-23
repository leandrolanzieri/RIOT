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
static void _encode_key_value(coral_element_t *e);

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
                       coral_form_target_t *target)
{
    form->type = CORAL_TYPE_FORM;
    form->next = NULL;
    form->children = NULL;
    form->children_n = 0;
    form->parent = NULL;
    form->v.form.method = method;
    form->v.form.op_type.str = op;
    form->v.form.op_type.len = strlen(op);
    memcpy(&form->v.form.target, target, sizeof(coral_form_target_t));
}

void coral_create_form_field(coral_element_t *field, char *type,
                             coral_literal_t *value)
{
    _init_element(field);
    field->type = CORAL_TYPE_FORM_FIELD;
    field->v.field.name.str = type;
    field->v.field.name.len = strlen(type);
    memcpy(&field->v.field.val, value, sizeof(coral_literal_t));
}

void coral_create_rep_metadata(coral_element_t *metadata, char *name,
                               coral_literal_t *value)
{
    assert(metadata && name && value);
    _init_element(metadata);
    metadata->type = CORAL_TYPE_REP_METADATA;
    metadata->v.rep_m.name.str = name;
    metadata->v.rep_m.name.len = strlen(name);
    memcpy(&metadata->v.rep_m.val, value, sizeof(coral_literal_t));
}

void coral_create_rep(coral_element_t *rep, uint8_t *buf, size_t buf_len)
{
    assert(rep && buf);
    _init_element(rep);
    rep->type = CORAL_TYPE_REP;
    rep->v.rep.bytes = buf;
    rep->v.rep.bytes_len = buf_len;
}

int coral_append_element(coral_element_t *root, coral_element_t *el)
{
    /* can only append fields to forms */
    if (root->type == CORAL_TYPE_FORM && el->type != CORAL_TYPE_FORM_FIELD) {
        DEBUG("Error, only append form fields to forms\n");
        return -1;
    }

    /* can only append metadata to embedded representations */
    if (root->type == CORAL_TYPE_REP && el->type != CORAL_TYPE_REP_METADATA) {
        DEBUG("Error, only append metadata to representations\n");
        return -1;
    }

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

    if (e->type != CORAL_TYPE_BODY && e->type != CORAL_TYPE_REP_METADATA) {
        e->cbor_root = cn_cbor_array_create(&_ct, NULL);
    }

    switch (e->type) {
        case CORAL_TYPE_LINK:
            DEBUG("Encoding link\n");
            _encode_link(e);
            break;
        case CORAL_TYPE_FORM:
            DEBUG("Encoding form\n");
            _encode_form(e);
            break;
        case CORAL_TYPE_REP:
            DEBUG("Encoding rep\n");
            _encode_rep(e);
            break;
        case CORAL_TYPE_FORM_FIELD:
        case CORAL_TYPE_REP_METADATA:
            DEBUG("Encoding key / value\n");
            _encode_key_value(e);
            return;
        case CORAL_TYPE_BODY:
            break;
        default:
            DEBUG("Unknown coral element\n");
    }

    if (e->children) {
        puts("Element has children");
        e->cbor_body = cn_cbor_array_create(&_ct, NULL);
        cn_cbor_array_append(e->cbor_root, e->cbor_body, NULL);
    }

    if (e->parent && e->parent->cbor_body) {
        cn_cbor_array_append(e->parent->cbor_body, e->cbor_root, NULL);
    }
}

static void _encode_literal(coral_literal_t *literal, cn_cbor **cb)
{

    switch (literal->type) {
        case CORAL_LITERAL_TEXT:
            *cb = cn_cbor_string_create(literal->v.as_str.str, &_ct, NULL);
            break;
        case CORAL_LITERAL_BOOL:
            *cb = cn_cbor_int_create(0, &_ct, NULL);
            (*cb)->type = literal->v.as_int ? CN_CBOR_TRUE : CN_CBOR_FALSE;
            break;
        case CORAL_LITERAL_BYTES:
            *cb = cn_cbor_data_create(literal->v.as_bytes.bytes,
                                      literal->v.as_bytes.bytes_len, &_ct,
                                      NULL);
            break;
        case CORAL_LITERAL_INT:
            *cb = cn_cbor_int_create(literal->v.as_int, &_ct, NULL);
            break;
        case CORAL_LITERAL_FLOAT:
            *cb = cn_cbor_float_create(literal->v.as_float, &_ct, NULL);
            break;
        default:
            DEBUG("Invalid literal type");
            return;
    }
}

static void _encode_link(coral_element_t *e)
{
    cn_cbor *val = cn_cbor_int_create(CORAL_TYPE_LINK, &_ct, NULL);
    cn_cbor_array_append(e->cbor_root, val, NULL);

    val = cn_cbor_string_create(e->v.link.rel_type.str, &_ct, NULL);
    cn_cbor_array_append(e->cbor_root, val, NULL);

    _encode_literal(&e->v.link.target, &val);
    cn_cbor_array_append(e->cbor_root, val, NULL);
}

static void _encode_key_value(coral_element_t *e)
{
    cn_cbor *val = cn_cbor_string_create(e->v.field.name.str, &_ct, NULL);
    e->cbor_root = val;

    if (e->parent && e->parent->cbor_body) {
        cn_cbor_array_append(e->parent->cbor_body, val, NULL);
        _encode_literal(&e->v.field.val, &val);
        cn_cbor_array_append(e->parent->cbor_body, val, NULL);
    }

}

static void _encode_form(coral_element_t *e)
{
    cn_cbor *val = cn_cbor_int_create(CORAL_TYPE_FORM, &_ct, NULL);
    cn_cbor_array_append(e->cbor_root, val, NULL);

    val = cn_cbor_string_create(e->v.form.op_type.str, &_ct, NULL);
    cn_cbor_array_append(e->cbor_root, val, NULL);

    val = cn_cbor_int_create(e->v.form.method, &_ct, NULL);
    cn_cbor_array_append(e->cbor_root, val, NULL);

    _encode_literal(&e->v.form.target, &val);
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


static void _print_literal(coral_literal_t *l)
{
    switch (l->type) {
        case CORAL_LITERAL_BOOL:
            DEBUG("%s", l->v.as_int ? "true" : "false");
            break;
        case CORAL_LITERAL_BYTES:
            for (unsigned i = 0; i < l->v.as_bytes.bytes_len; i++) {
                DEBUG("%02x ", l->v.as_bytes.bytes[i]);
            }
            break;
        case CORAL_LITERAL_TEXT:
            DEBUG("\"%.*s\"", l->v.as_str.len, l->v.as_str.str);
            break;
        case CORAL_LITERAL_NULL:
            DEBUG("NULL");
            break;
        case CORAL_LITERAL_TIME:
            DEBUG("Time format not implemented");
            break;
        case CORAL_LITERAL_FLOAT:
            DEBUG("%f", l->v.as_float);
            break;
        default:
            DEBUG("Unknown literal type");
            break;
    }
}

static void _print_visited(coral_element_t *e, int depth, void *context)
{
    (void)context;
    DEBUG("|--");
    for (int i = 0; i < depth; i++) {
        DEBUG("|--");
    }
    DEBUG("|(%d children)", e->children_n);
    DEBUG(" TYPE: ");

    switch (e->type) {
        case CORAL_TYPE_LINK:
            DEBUG("link - rel: %.*s target: ", e->v.link.rel_type.len,
                   e->v.link.rel_type.str);
            _print_literal(&e->v.link.target);
            break;

        case CORAL_TYPE_FORM:
            DEBUG("form - op: %.*s method: %d target: ", e->v.form.op_type.len,
                   e->v.form.op_type.str, e->v.form.method);
            _print_literal(&e->v.form.target);
            break;

        case CORAL_TYPE_FORM_FIELD:
            DEBUG("form field - type: %.*s value: ", e->v.field.name.len,
                  e->v.field.name.str);
            _print_literal(&e->v.field.val);
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
        case CORAL_TYPE_REP_METADATA:
            DEBUG("representation metadata - name: %.*s value: ",
                  e->v.rep_m.name.len, e->v.rep_m.name.str);
            _print_literal(&e->v.field.val);
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
        p = p->children;
        depth++;
      } else{
        if (p->next) {
          p = p->next;
        } else {
          while (p->parent) {
            depth--;
            if (p->parent->next) {
              p = p->parent->next;
              goto visit;
            }
            p = p->parent;
          }
          return;
        }
      }
    }
}

static int _decode_literal(coral_literal_t *literal, cn_cbor *cb)
{
    switch (cb->type) {
        case CN_CBOR_TEXT:
            literal->type = CORAL_LITERAL_TEXT;
            literal->v.as_str.str = cb->v.str;
            literal->v.as_str.len = cb->length;
            break;
        case CN_CBOR_TRUE:
        case CN_CBOR_FALSE:
            literal->type = CORAL_LITERAL_BOOL;
            literal->v.as_int = (cb->type == CN_CBOR_TRUE);
            break;
        case CN_CBOR_BYTES:
            literal->type = CORAL_LITERAL_BYTES;
            literal->v.as_bytes.bytes = cb->v.bytes;
            literal->v.as_bytes.bytes_len = cb->length;
            break;
        case CN_CBOR_INT:
            literal->type = CORAL_LITERAL_INT;
            literal->v.as_int = cb->v.uint;
            break;
        case CN_CBOR_FLOAT:
            literal->type = CORAL_LITERAL_FLOAT;
            literal->v.as_float = cb->v.f;
            break;
        default:
            puts("Literal not valid");
            return -1;
    }
    return 0;
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
    if (!p) {
        DEBUG("Error, link has no target\n");
        return -1;
    }
    _decode_literal(&ctx->current->v.link.target, p);

    DEBUG("We found a link:\n");
    DEBUG("- Rel: %.*s\n", ctx->current->v.link.rel_type.len, ctx->current->v.link.rel_type.str);
    DEBUG("Target: ");
    _print_literal(&ctx->current->v.link.target);

    coral_append_element(ctx->parent, ctx->current);

    /* if has a body point to it */
    if (p->next && p->next->type == CN_CBOR_ARRAY) {
        DEBUG("Link has a body\n");
        *body = p->next->first_child;
    }
    DEBUG("\n");
    return 0;
}

static int _decode_rep_metadata(cn_cbor **cb, _decode_ctx_t *ctx)
{
    cn_cbor *p;
    ctx->current = _get_from_element_pool(ctx->pool);
    _init_element(ctx->current);
    ctx->current->parent = ctx->parent;
    ctx->current->type = CORAL_TYPE_REP_METADATA;

    /* metadata name */
    p = *cb;
    if (!p || p->type != CN_CBOR_TEXT) {
        puts("here");
        DEBUG("Error, representation metadata name should be text."
              "Type is: %d\n", p->type);
        return -1;
    }

    ctx->current->v.rep_m.name.str = p->v.str;
    ctx->current->v.rep_m.name.len = p->length;

    /* metdata value */
    p = p->next;
    if (!p) {
        DEBUG("Error, metadata has no value\n");
        return -1;
    }
    _decode_literal(&ctx->current->v.rep_m.val, p);
    coral_append_element(ctx->parent, ctx->current);
    puts("added metadata to parent");
    *cb = p->next;

    DEBUG("We found a rep metadata:\n");
    DEBUG("- Name: %.*s\n", ctx->current->v.rep_m.name.len,
          ctx->current->v.rep_m.name.str);
    DEBUG("- Value: ");
    _print_literal(&ctx->current->v.rep_m.val);

    return 0;
}

static int _decode_form_field(cn_cbor *cb, _decode_ctx_t *ctx)
{
    cn_cbor *p;
    ctx->current = _get_from_element_pool(ctx->pool);
    _init_element(ctx->current);
    ctx->current->parent = ctx->parent;
    ctx->current->type = CORAL_TYPE_FORM_FIELD;

    /* form field type */
    p = cb->first_child;
    if (!p || p->type != CN_CBOR_TEXT) {
        DEBUG("Error, form field type should be text. Type is: %d\n", p->type);
        return -1;
    }

    ctx->current->v.field.name.str = p->v.str;
    ctx->current->v.field.name.len = p->length;

    /* form value */
    p = p->next;
    if (!p) {
        DEBUG("Error, form field has no value\n");
        return -1;
    }
    _decode_literal(&ctx->current->v.field.val, p);
    coral_append_element(ctx->parent, ctx->current);

    DEBUG("We found a form field:\n");
    DEBUG("- Type: %.*s\n", ctx->current->v.field.name.len, ctx->current->v.field.name.str);
    DEBUG("Target: ");
    _print_literal(&ctx->current->v.field.val);

    return 0;
}

static int _decode_form(cn_cbor *cb, cn_cbor **body, _decode_ctx_t *ctx)
{
    (void)body;
    cn_cbor *p;
    *body = NULL;
    coral_element_t *e = _get_from_element_pool(ctx->pool);
    coral_element_t *parent = ctx->parent;
    ctx->current = e;
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
    if (!p) {
        DEBUG("Error, form target not present\n");
        return -1;
    }
    _decode_literal(&ctx->current->v.link.target, p);

    /* check if there are fields */
    p = p->next;
    if (!p) {
        DEBUG("Form has no fields\n");
    }
    else {
        if (p->type != CN_CBOR_ARRAY) {
            DEBUG("Error, form fields should be in an array. Type: %d\n", p->type);
            return -1;
        }
        p = p->first_child;
        ctx->parent = ctx->current;
        DEBUG("Form has fields\n");
        while (p) {
            _decode_form_field(p, ctx);
            p = p->next;
        }

        ctx->current = e;
        ctx->parent = parent;
    }

    DEBUG("We found a form:\n");
    DEBUG("- Op type: %.*s\n", ctx->current->v.form.op_type.len, ctx->current->v.form.op_type.str);
    DEBUG("- Method: %d\n", ctx->current->v.form.method);
    DEBUG("- Target: ");
    _print_literal(&ctx->current->v.form.target);
    DEBUG("\n");

    coral_append_element(ctx->parent, ctx->current);
    return 0;
}

static int _decode_rep(cn_cbor *cb, cn_cbor **body, _decode_ctx_t *ctx)
{
    cn_cbor *p;
    coral_element_t *e = _get_from_element_pool(ctx->pool);
    coral_element_t *parent = ctx->parent;
    ctx->current = e;
    *body = NULL; /* metadata is parsed here */
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
    DEBUG("\n");
    coral_append_element(ctx->parent, ctx->current);

    /* check if there is metadata */
    p = p->next;
    if (!p) {
        DEBUG("Representation has no metadata\n");
    }
    else {
        if (p->type != CN_CBOR_ARRAY) {
            DEBUG("Error, emb rep metadata should be an array. Type: %d\n", p->type);
            return -1;
        }
        p = p->first_child;
        ctx->parent = ctx->current;
        DEBUG("Representation has metadata\n");
        while (p) {
            DEBUG("Decoding metadata\n");
            _decode_rep_metadata(&p, ctx);
        }

        ctx->current = e;
        ctx->parent = parent;
    }

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

    printf("CBOR root: %p\n", (void*)cb);
    while (p) {
    cn_cbor *res = NULL;
element:
        if (p == cb) {
            DEBUG("Back to CBOR root, done decoding\n");
            return 0;
        }
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
                p = p->next;
            }
            else {
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

