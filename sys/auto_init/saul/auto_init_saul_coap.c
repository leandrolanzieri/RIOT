//#ifdef MODULE_SAUL_COAP

#include "fmt.h"
#include "net/gcoap.h"
#include "saul_reg.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#ifndef SAUL_COAP_PATH
#define SAUL_COAP_PATH "/saul"
#endif

static ssize_t _handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx);
static ssize_t _encode_reg(saul_reg_t *reg, uint8_t *buf, size_t len);
static int _decode_payload(coap_pkt_t *pdu, phydat_t *data);

static const coap_resource_t _resource = {
    SAUL_COAP_PATH, COAP_GET | COAP_POST | COAP_MATCH_SUBTREE, _handler, NULL
};
static gcoap_listener_t _listener = { &_resource, 1, NULL };

static ssize_t _handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    size_t resp_len = 0;
    saul_reg_t *reg = saul_reg;

    uint8_t uri[NANOCOAP_URI_MAX];
    coap_get_uri_path(pdu, uri);
    unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));

    if (!reg) {
        goto empty_out;

    }

    if (strlen((char *)uri) == sizeof(SAUL_COAP_PATH) - 1) {
        gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
        coap_opt_add_format(pdu, COAP_FORMAT_LINK);
        resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
    
        size_t pos = 0;
        while(reg) {
            if (pos) {
                if (pos + 1 > pdu->payload_len) {
                    goto out;
                }
                pdu->payload[pos++] = ',';
            }
            ssize_t res = _encode_reg(reg, &pdu->payload[pos], pdu->payload_len - pos);
            if (res < 0) {
                goto out;
            }
            pos += res;
            reg = reg->next;
        }
        resp_len += pos;
        goto out;
    }
    else {
        char *dev_uri = (char *)&uri[sizeof(SAUL_COAP_PATH)];
        reg = saul_reg_find_name(dev_uri);
        if (!reg) {
            gcoap_resp_init(pdu, buf, len, COAP_CODE_PATH_NOT_FOUND);
            goto empty_out;
        }

        phydat_t data;
        int res;
        size_t pos = 0;

        switch (method_flag) {
            case COAP_GET:
                res = reg->driver->read(reg->dev, &data);
                if (res < 0) {
                    gcoap_resp_init(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
                    goto empty_out;
                }
                gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
                resp_len = coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
                resp_len += coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

                for (int i = 0; i < res; i++) {
                    pos += fmt_s16_dec((char *)&pdu->payload[pos], data.val[i]);
                }

                resp_len += pos;
                goto out;
            case COAP_POST:
                _decode_payload(pdu, &data);
                res = reg->driver->write(reg->dev, &data);
                gcoap_resp_init(pdu, buf, len, COAP_CODE_CHANGED);
                goto empty_out;
            default:
                DEBUG("other method: %d\n", method_flag);
        }
        goto out;
    }

empty_out:
    resp_len += coap_opt_finish(pdu, COAP_OPT_FINISH_NONE);
out:
    return resp_len;
}

static ssize_t _encode_reg(saul_reg_t *reg, uint8_t *buf, size_t len)
{
    size_t name_len = strlen(reg->name);
    size_t pos = 0;
    if (name_len + 3 + sizeof(SAUL_COAP_PATH) > len) {
        return -1;
    }

    buf[pos++] = '<';
    strcpy((char *)&buf[pos], SAUL_COAP_PATH);
    pos += sizeof(SAUL_COAP_PATH) - 1;
    buf[pos++] = '/';
    memcpy(&buf[pos], reg->name, name_len);
    pos += name_len;
    buf[pos++] = '>';
    return pos;
}

static int _decode_payload(coap_pkt_t *pdu, phydat_t *data)
{
    data->val[0] = scn_u32_dec((char *)pdu->payload, 5);
    return 0;
}

void auto_init_saul_coap(void)
{
    gcoap_register_listener(&_listener);
}
// #else
// typedef int dont_be_pedantic;
// #endif /* MODULE_SAUL_COAP */