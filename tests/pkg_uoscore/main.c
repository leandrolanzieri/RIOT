/*
 * Copyright (C) 2021 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @}
 */

#include "oscore.h"
#include "credentials.h"
#include "net/nanocoap.h"
#include "od.h"

#define ENABLE_DEBUG 1
#include "debug.h"

#define MAX_COAP_MSG_LEN 256

// uint8_t coap_packet[] = {
//     0x42, 0x02, 0x1f, 0x99, 0x32, 0xfb, 0xb4, 0x31, 0x32,
//     0x33, 0x34, 0xff, 0x55, 0x55, 0x55, 0x55
// };


/*
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |Ver| T |  TKL  |      Code     |          Message ID           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   Token (if any, TKL bytes) ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   Options (if any) ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |1 1 1 1 1 1 1 1|    Payload (if any) ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
uint8_t coap_packet[] = {
    0x44, 0x02, 0x97, 0xfa, 0xfa, 0x97, 0x03, 0x00, 0xb2, 0x72, 0x64, 0x11, 0x28, 0x39, 0x6c, 0x77,
    0x6d, 0x32, 0x6d, 0x3d, 0x31, 0x2e, 0x30, 0x0d, 0x04, 0x65, 0x70, 0x3d, 0x74, 0x65, 0x73, 0x74,
    0x52, 0x49, 0x4f, 0x54, 0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x03, 0x62, 0x3d, 0x55, 0x06, 0x6c,
    0x74, 0x3d, 0x33, 0x30, 0x30, 0xff, 0x3c, 0x2f, 0x3e, 0x3b, 0x72, 0x74, 0x3d, 0x22, 0x6f, 0x6d,
    0x61, 0x2e, 0x6c, 0x77, 0x6d, 0x32, 0x6d, 0x22, 0x2c, 0x3c, 0x2f, 0x31, 0x2f, 0x30, 0x3e, 0x2c,
    0x3c, 0x2f, 0x33, 0x2f, 0x30, 0x3e,
};

uint8_t oscore_packet [512] = {0};

int main(void)
{
    struct context oscore_context_client;
    struct context oscore_context_server;

    struct oscore_init_params oscore_params_client = {
        .dev_type = CLIENT,
        .master_secret.ptr = MASTER_SECRET,
        .master_secret.len = MASTER_SECRET_LEN,
        .sender_id.ptr = CLIENT_SENDER_ID,
        .sender_id.len = CLIENT_SENDER_ID_LEN,
        .recipient_id.ptr = CLIENT_RECIPIENT_ID,
        .recipient_id.len = CLIENT_RECIPIENT_ID_LEN,
        .id_context.ptr = ID_CONTEXT,
        .id_context.len = ID_CONTEXT_LEN,
        .master_salt.ptr = MASTER_SALT,
        .master_salt.len = MASTER_SALT_LEN,
        .aead_alg = AES_CCM_16_64_128,
        .hkdf = SHA_256,
    };

    struct oscore_init_params oscore_params_server = {
        .dev_type = SERVER,
        .master_secret.ptr = MASTER_SECRET,
        .master_secret.len = MASTER_SECRET_LEN,
        .sender_id.ptr = SERVER_SENDER_ID,
        .sender_id.len = SERVER_SENDER_ID_LEN,
        .recipient_id.ptr = SERVER_RECIPIENT_ID,
        .recipient_id.len = SERVER_RECIPIENT_ID_LEN,
        .id_context.ptr = ID_CONTEXT,
        .id_context.len = ID_CONTEXT_LEN,
        .master_salt.ptr = MASTER_SALT,
        .master_salt.len = MASTER_SALT_LEN,
        .aead_alg = AES_CCM_16_64_128,
        .hkdf = SHA_256,
    };

    int res = oscore_context_init(&oscore_params_client, &oscore_context_client);

    res = oscore_context_init(&oscore_params_server, &oscore_context_server);

    if (res != OscoreNoError) {
        DEBUG("Error during establishing an OSCORE security context!\n");
    } else {
        DEBUG("OSCORE security contexts successfully initialized\n");
    }

    uint16_t oscore_packet_len = sizeof(oscore_packet);
    res = coap2oscore(coap_packet, sizeof(coap_packet), oscore_packet, &oscore_packet_len,
                      &oscore_context_client);

    if (res != OscoreNoError) {
        DEBUG("Error during establishing converting CoAP to OSCORE\n");
        return 1;
    }

    DEBUG("Original CoAP Packet: \n");
    od_hex_dump(coap_packet, sizeof(coap_packet), 0);

    DEBUG("OSCORE Packet: \n");
    od_hex_dump(oscore_packet, oscore_packet_len, 0);

    uint8_t decrypted_oscore[MAX_COAP_MSG_LEN];
    uint16_t decrypted_oscore_len = 0;
    bool is_oscore = false;

    res = oscore2coap(oscore_packet, oscore_packet_len, decrypted_oscore, &decrypted_oscore_len,
                      &is_oscore, &oscore_context_server);

    if (res != OscoreNoError) {
        DEBUG("Could not decode the OSCORE packet (res=%d)\n", res);
        return 1;
    }

    if (is_oscore) {
        DEBUG("Found an OSCORE packet\n");
        od_hex_dump(decrypted_oscore, decrypted_oscore_len, 0);
    }
    else {
        DEBUG("No OSCORE packet found\n");
    }

    if (decrypted_oscore_len != sizeof(coap_packet) || memcmp(decrypted_oscore, coap_packet, sizeof(coap_packet))) {
        printf("ERROR: Decrypted packet does not match original\n");
    }
    else {
        printf("PASS\n");
    }

    return 0;
}
