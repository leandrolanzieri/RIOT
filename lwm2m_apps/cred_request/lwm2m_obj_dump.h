#include "lwm2m_client.h"

/**
 * @brief Dumps client objects to STDIO.
 *
 * @param[in] client    Client to dump objects from.
 * @param[in] obj_id    ID of the object to dump. Set to <0 to dump all.
 */
void dump_client_objects(lwm2m_client_data_t *client, int obj_id);
