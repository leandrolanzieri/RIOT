#include "fmt.h"
#include "net/gcoap.h"
#include "saul_reg.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#ifndef SAUL_COAP_PATH
#define SAUL_COAP_PATH "/saul"
#endif

static ssize_t _handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx);
static ssize_t _encode_dev(saul_reg_t *dev, uint8_t *buf, size_t len);

static const coap_resource_t _resource = {
    SAUL_COAP_PATH, COAP_GET | COAP_POST | COAP_MATCH_SUBTREE, _handler, NULL
};
static gcoap_listener_t _listener = { &_resource, 1, NULL };

static ssize_t _handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    size_t resp_len = 0;
    saul_reg_t *dev = saul_reg;

    uint8_t uri[NANOCOAP_URI_MAX];
    coap_get_uri_path(pdu, uri);
    unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));

    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    if (!strcmp((char *)uri, SAUL_COAP_PATH)) {
        coap_opt_add_format(pdu, COAP_FORMAT_LINK);
        resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
    
        size_t pos = 0;
        while(dev) {
            if (pos) {
                if (pos + 1 > pdu->payload_len) {
                    goto out;
                }
                pdu->payload[pos++] = ',';
            }
            ssize_t res = _encode_dev(dev, &pdu->payload[pos], pdu->payload_len - pos);
            if (res < 0) {
                goto out;
            }
            pos += res;
            dev = dev->next;
        }
        resp_len += pos;
        DEBUG("Pos: %d\n", pos);
        DEBUG("Resp len: %d\n", resp_len);
    }
    else {
        char *dev_uri = (char *)&uri[sizeof(SAUL_COAP_PATH)];
        DEBUG("Getting resource: %s\n", dev_uri);
        while (dev) {
            if (!strcmp(dev_uri, dev->name)) {
                break;
            }
        }
        if (!dev) {
            goto out;
        }

        phydat_t data;
        int res;
        size_t pos = 0;

        switch (method_flag) {
            case COAP_GET:
                res = dev->driver->read(dev, &data);
                if (res < 0) {
                    goto empty_out;
                }
                resp_len = coap_opt_add_format(pdu, COAP_FORMAT_LINK);
                resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

                for (int i = 0; i < res; i++) {
                    pos += fmt_s16_dec((char *)&pdu->payload[pos], data.val[i]);
                }
                resp_len += pos;
                break;
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

static ssize_t _encode_dev(saul_reg_t *dev, uint8_t *buf, size_t len)
{
    size_t name_len = strlen(dev->name);
    if (name_len + 2 > len) {
        return -1;
    }
    buf[0] = '<';
    memcpy(&buf[1], dev->name, name_len);
    buf[name_len + 1] = '>';
    return name_len + 2;
}

void auto_init_saul_coap(void)
{
    gcoap_register_listener(&_listener);
}
