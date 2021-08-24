#ifndef OSCORE_CREDS_H
#define OSCORE_CREDS_H

#ifdef __cplusplus
extern "C" {
#endif

/* configured from Kconfig */
static const char master_salt[] = CONFIG_OSCORE_MASTER_SALT;
static const char master_secret[] = CONFIG_OSCORE_MASTER_SECRET;
static const char recipient_id[] = CONFIG_OSCORE_RECIPIENT_ID;
static const char sender_id[] = CONFIG_OSCORE_SENDER_ID;

/* instance number to store the client OSCORE information */
#define OSCORE_OBJECT_INSTANCE_ID   (0)

#ifdef __cplusplus
}
#endif

#endif /* OSCORE_CREDS_H */
