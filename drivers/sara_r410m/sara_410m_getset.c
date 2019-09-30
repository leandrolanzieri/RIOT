#include <string.h>

#include "fmt.h"
#include "sara_r410m.h"
#include "sara_r410m_params.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

int sara_r410m_set_band(sara_r410m_t *dev, uint16_t band)
{
    char bitmask[21];
    const char cmd[] = "AT+UBANDMASK=1,";

    at_send_bytes(&dev->at, cmd, sizeof(cmd) - 1);
    memset(bitmask, 0, sizeof(bitmask));
    if (band > 64) {
        at_send_bytes(&dev->at, "0,", sizeof("0,") - 1);
        band -= 64;
    }
    fmt_u64_dec(bitmask, 1 << (band - 1));
    if (at_send_cmd_wait_ok(&dev->at, bitmask, SARA_R410M_STD_TIMEOUT)) {
        DEBUG("Could not set band\n");
        return SARA_R410M_ERR;
    }
    return SARA_R410M_OK;
}

int sara_r410m_set_apn(sara_r410m_t *dev, const char *apn)
{
    const char cmd[] = "AT+CGDCONT=1, \"IP\",\"";
    at_send_bytes(&dev->at, cmd, sizeof(cmd) - 1);
    at_send_bytes(&dev->at, apn, strlen(apn));

    if (at_send_cmd_wait_ok(&dev->at, "\"", SARA_R410M_STD_TIMEOUT)) {
        DEBUG("Could not set APN\n");
        return SARA_R410M_ERR;
    }
    return SARA_R410M_OK;
}

int sara_r410m_set_pin(sara_r410m_t *dev, const char *pin)
{
    /* Unlock SIM card if necessary */
    const char cmd[] = "AT+CPIN=";
    at_send_bytes(&dev->at, cmd, sizeof(cmd) - 1);
    if (pin) {
        at_send_bytes(&dev->at, "\"", 1);
        at_send_bytes(&dev->at, pin, strlen(pin));
        if (at_send_cmd_wait_ok(&dev->at, "\"", SARA_R410M_STD_TIMEOUT)) {
            DEBUG("Could not unlock SIM with PIN\n");
            goto error;
        }
    }
    else {
        if (at_send_cmd_wait_ok(&dev->at, "?", SARA_R410M_STD_TIMEOUT)) {
            DEBUG("Could not unlock SIM without PIN\n");
            goto error;
        }
    }
    return SARA_R410M_OK;

error:
    DEBUG("[SARA R410M] Error setting PIN\n");
    return SARA_R410M_ERR;
}

int sara_r410m_set_status(sara_r410m_t *dev, sara_r410m_status_t status)
{
    DEBUG("[SARA R410M] Setting status: %d\n", status);
    if (status != STATUS_MINIMUM && status != STATUS_ACTIVE) {
        // TODO: implement other modes
        DEBUG("[SARA R410M] Status not implemented\n");
        return SARA_R410M_ERR;
    }

    char _status = status == STATUS_ACTIVE ? '1' : '0';
    char msg[] = "AT+CFUN= ";

    msg[sizeof(msg) - 2] = _status;
    return at_send_cmd_wait_ok(&dev->at, msg, SARA_R410M_STD_TIMEOUT);
}
