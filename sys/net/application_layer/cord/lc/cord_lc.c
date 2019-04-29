/*
 * Copyright (C) 2017-2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_cord_lc
 * @{
 *
 * @file
 * @brief       CoRE Resource Directory lookup implementation
 *
 * @author      Aiman Ismail <muhammadaimanbin.ismail@haw-hamburg.de>
 *
 * @}
 */

#include <string.h>

#include "mutex.h"
#include "assert.h"
#include "thread_flags.h"

#include "net/gcoap.h"
#include "net/cord/lc.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define FLAG_SUCCESS        (0x0001)
#define FLAG_TIMEOUT        (0x0002)
#define FLAG_ERR            (0x0004)
#define FLAG_OVERFLOW       (0x0008)
#define FLAG_NORSC          (0x0010)
#define FLAG_MASK           (0x00ff)

#define BUFSIZE             (GCOAP_PDU_BUF_SIZE)

static char *_result_buf;
static size_t _result_buf_len;

static mutex_t _mutex = MUTEX_INIT;
static volatile thread_t *_waiter;

static void _lock(void)
{
    mutex_lock(&_mutex);
    _waiter = sched_active_thread;
}

static int _sync(void)
{
    thread_flags_t flags = thread_flags_wait_any(FLAG_MASK);

    if (flags & FLAG_ERR) {
        return CORD_LC_ERR;
    }
    else if (flags & FLAG_TIMEOUT) {
        return CORD_LC_TIMEOUT;
    }
    else if (flags & FLAG_OVERFLOW) {
        return CORD_LC_OVERFLOW;
    }
    else if (flags & FLAG_NORSC) {
        return CORD_LC_NORSC;
    }
    else {
        return CORD_LC_OK;
    }
}

static void _on_lookup(unsigned req_state, coap_pkt_t *pdu,
                       sock_udp_ep_t *remote)
{
    thread_flags_t flag = FLAG_ERR;
    (void)remote;

    if (req_state == GCOAP_MEMO_RESP) {
        unsigned ct = coap_get_content_type(pdu);
        if (ct != COAP_FORMAT_LINK) {
            goto end;
        }
        if (pdu->payload_len == 0) {
            flag = FLAG_NORSC;
            goto end;
        }
        if (pdu->payload_len >= _result_buf_len) {
            flag = FLAG_OVERFLOW;
            goto end;
        }
        memcpy(_result_buf, pdu->payload, pdu->payload_len);
        memset(_result_buf + pdu->payload_len, 0, _result_buf_len - pdu->payload_len);
        _result_buf_len = pdu->payload_len;
        flag = FLAG_SUCCESS;
    }
    else if (req_state == GCOAP_MEMO_TIMEOUT) {
        flag = FLAG_TIMEOUT;
    }

end:
    thread_flags_set((thread_t *)_waiter, flag);
}

static ssize_t _lookup_raw(const cord_lc_rd_t *rd, unsigned content_format,
                           unsigned lookup_type, cord_lc_filter_t *filters,
                           void *result, size_t maxlen)
{
    assert(rd->remote);
    uint8_t buf[GCOAP_PDU_BUF_SIZE];
    memset(buf, 0, sizeof(buf));

    int res;
    int retval;
    coap_pkt_t pkt;
    ssize_t pkt_len;

    char lookif[NANOCOAP_QS_MAX];
    memset(lookif, 0, sizeof(lookif));
    switch (lookup_type) {
    case CORD_LC_RES:
        memcpy(lookif, rd->res_lookif, rd->res_lookif_len);
        break;
    case CORD_LC_EP:
        memcpy(lookif, rd->ep_lookif, rd->ep_lookif_len);
        break;
    }

    res = gcoap_req_init(&pkt, buf, sizeof(buf), COAP_METHOD_GET, lookif);
    if (res < 0) {
        retval = CORD_LC_ERR;
        DEBUG("cord_lc: failed gcoap_req_init()\n");
        goto end;
    }


    /* save pointer to result */
    _result_buf = result;
    _result_buf_len = maxlen;

    /* add filters */
    cord_lc_filter_t *f = filters;
    while (f) {
        for (unsigned i = 0; i < f->len; i++) {
	    char key[32];
	    char val[32];
	    snprintf(key, f->array[i].key_len + 1, "%s", f->array[i].key);
	    snprintf(val, f->array[i].value_len + 1, "%s", f->array[i].value);
            gcoap_add_qstring(&pkt, key, val);
        }
        f = f->next;
    }

    /* set packet options */
    coap_hdr_set_type(pkt.hdr, COAP_TYPE_CON);
    coap_opt_add_uint(&pkt, COAP_OPT_ACCEPT, content_format);
    // TODO: handle 4.15 error (unsupported content format)

    pkt_len = gcoap_finish(&pkt, 0, COAP_FORMAT_NONE);
    if (pkt_len < 0) {
        retval = CORD_LC_ERR;
        goto end;
    }
    res = gcoap_req_send2(buf, pkt_len, rd->remote, _on_lookup);
    if (res < 0) {
        retval = CORD_LC_ERR;
        goto end;
    }
    retval = _sync();
end:
    return (retval == CORD_LC_OK) ? (int) _result_buf_len : retval;
}

static void _on_rd_init(unsigned req_state, coap_pkt_t *pdu,
                        sock_udp_ep_t *remote)
{
    thread_flags_t flag = FLAG_NORSC;
    (void)remote;

    if (req_state == GCOAP_MEMO_RESP) {
        unsigned ct = coap_get_content_type(pdu);
        if (ct != COAP_FORMAT_LINK) {
            DEBUG("cord_lc: error payload not in link format: %u\n", ct);
            goto end;
        }
        if (pdu->payload_len == 0) {
            DEBUG("cord_lc: error empty payload\n");
            goto end;
        }
        /* memmove ? */
        memcpy(_result_buf, pdu->payload, pdu->payload_len);
        _result_buf_len = pdu->payload_len;
        memset((_result_buf + pdu->payload_len), 0,
               (_result_buf_len - pdu->payload_len));
        flag = FLAG_SUCCESS;
    }
    else if (req_state == GCOAP_MEMO_TIMEOUT) {
        flag = FLAG_TIMEOUT;
    }

end:
    if (flag != FLAG_SUCCESS) {
        _result_buf = NULL;
        _result_buf_len = 0;
    }
    thread_flags_set((thread_t *)_waiter, flag);
}

static int _send_rd_init_req(coap_pkt_t *pkt, const sock_udp_ep_t *remote,
                             void *buf, size_t maxlen)
{
    int res = gcoap_req_init(pkt, buf, maxlen, COAP_METHOD_GET,
                             "/.well-known/core");
    if (res < 0) {
        DEBUG("cord_lc: error gcoap_req_init() %d\n", res);
        return CORD_LC_ERR;
    }

    coap_hdr_set_type(pkt->hdr, COAP_TYPE_CON);
    gcoap_add_qstring(pkt, "rt", "core.rd-lookup-*");

    ssize_t pkt_len = gcoap_finish(pkt, 0, COAP_FORMAT_NONE);
    if (pkt_len < 0) {
        DEBUG("cord_lc: error gcoap_finish() %d\n", pkt_len);
        return CORD_LC_ERR;
    }

    res = gcoap_req_send2(buf, pkt_len, remote, _on_rd_init);
    if (!res) {
        DEBUG("cord_lc: error gcoap_req_send2() %d\n", res);
        return CORD_LC_ERR;
    }
    return _sync();
}

int cord_lc_rd_init(cord_lc_rd_t *rd, const sock_udp_ep_t *remote,
                    void *buf, size_t maxlen)
{
    assert(remote);

    rd->remote = remote;
    int retval = CORD_LC_OK;
    coap_pkt_t pkt;

    _lock();
    memset(buf, '0', maxlen);
    _result_buf = buf;
    _result_buf_len = maxlen;

    retval = _send_rd_init_req(&pkt, remote, buf, maxlen);
    if (retval != CORD_LC_OK) {
        DEBUG("cord_lc: failed to send req %d\n", retval);
        goto end;
    }

    /* Parse the payload */
    clif_t lookif;
    clif_param_t params[6];
    unsigned params_used = 0;
    size_t parsed_len = 0;
    while ((!rd->res_lookif || !rd->ep_lookif) || (parsed_len != _result_buf_len)) {
        ssize_t ret = clif_decode_link(&lookif,
                          params + params_used,
                          (sizeof(params) / sizeof(clif_param_t)) - params_used,
                          ((char *) _result_buf) + parsed_len,
                          _result_buf_len - parsed_len);
        if (ret < 0) {
            DEBUG("cord_lc: error decoding payload %d\n", ret);
            retval = CORD_LC_ERR;
            goto end;
        }

        parsed_len += ret;
        /* check if we found ep_lookif or res_lookif */
        for (unsigned i = 0; i < lookif.params_len; i++) {
            if (!strncmp(params[i + params_used].value,
                         "core.rd-lookup-res",
                         params[i + params_used].value_len)) {
                rd->res_lookif = lookif.target;
                rd->res_lookif_len = lookif.target_len;
            }
            else if (!strncmp(params[i + params_used].value,
                              "core.rd-lookup-ep",
                              params[i + params_used].value_len)) {
                rd->ep_lookif = lookif.target;
                rd->ep_lookif_len = lookif.target_len;
            }
        }
        params_used += lookif.params_len;
    }

    if (!rd->res_lookif && !rd->ep_lookif) {
        DEBUG("cord_lc: no lookup interfaces found\n");
        retval = CORD_LC_NORSC;
        goto end;
    }
    else {
        retval = parsed_len;
        goto end;
    }
end:
    mutex_unlock(&_mutex);
    return retval;
}

ssize_t cord_lc_raw(const cord_lc_rd_t *rd, unsigned content_format,
                    unsigned lookup_type, cord_lc_filter_t *filters,
                    void *result, size_t maxlen)
{
    _lock();
    ssize_t retval = _lookup_raw(rd, content_format, lookup_type, filters,
                                 result, maxlen);
    mutex_unlock(&_mutex);
    return retval;
}

ssize_t cord_lc_res(cord_lc_rd_t *rd, cord_lc_res_t *resource,
                    cord_lc_filter_t *filters, void* buf, size_t maxlen)
{
    int retval;

    _lock();

    char page[3];
    ssize_t pagelen = snprintf(page, maxlen, "%u", rd->res_last_page++);
    clif_param_t new_array[] = {
        {
            .key = "count", .key_len = sizeof("count"),
            .value = "1", .value_len = sizeof("1"),
        },
        {
            .key = "page", .key_len = sizeof("page"),
            .value = page, .value_len = pagelen,
        },
    };
    cord_lc_filter_t new_filters;
    new_filters.array = new_array;
    new_filters.len = sizeof(new_array) / sizeof(clif_param_t);
    new_filters.next = NULL;
    filters->next = &new_filters;

    /* defaults to application/link-format (ct=40) for the content format */
    retval = _lookup_raw(rd, COAP_FORMAT_LINK, CORD_LC_RES, filters, buf, maxlen);
    if (retval < 0) {
        if (retval == CORD_LC_NORSC) {
            rd->res_last_page = 0;
        }
        DEBUG("cord_lc: error res lookup failed\n");
        goto end;
    }

    /* parse the result */
    retval = clif_decode_link(&resource->link, resource->params,
                              resource->max_params, buf, retval);
    if (retval < 0) {
        DEBUG("cord_lc: no resource link found\n");
        goto end;
    }


end:
    mutex_unlock(&_mutex);
    return retval;
}

ssize_t cord_lc_ep(cord_lc_rd_t *rd, cord_lc_ep_t *endpoint,
                   cord_lc_filter_t *filters, void* buf, size_t maxlen)
{
    int retval;

    _lock();

    char page[3];
    ssize_t pagelen = snprintf(page, maxlen, "%u", rd->ep_last_page++);
    clif_param_t new_array[] = {
        {
            .key = "count", .key_len = sizeof("count"),
            .value = "1", .value_len = sizeof("1"),
        },
        {
            .key = "page", .key_len = sizeof("page"),
            .value = page, .value_len = pagelen,
        },
    };
    cord_lc_filter_t new_filters;
    new_filters.array = new_array;
    new_filters.len = sizeof(new_array) / sizeof(clif_param_t);
    new_filters.next = NULL;
    filters->next = &new_filters;

    /* defaults to application/link-format (ct=40) for the content format */
    retval = _lookup_raw(rd, COAP_FORMAT_LINK, CORD_LC_EP, filters, buf, maxlen);
    if (retval < 0) {
        if (retval == CORD_LC_NORSC) {
            rd->ep_last_page = 0;
        }
        DEBUG("cord_lc: error ep lookup failed\n");
        goto end;
    }

    /* parse the result */
    retval = clif_decode_link(&endpoint->link, endpoint->params,
                              endpoint->max_params, buf, retval);
    if (retval < 0) {
        DEBUG("cord_lc: no endpoint link found\n");
        goto end;
    }

end:
    mutex_unlock(&_mutex);
    return retval;
}
