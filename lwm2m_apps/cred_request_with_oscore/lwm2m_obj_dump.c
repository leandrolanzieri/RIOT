#include <stdio.h>

#include "lwm2m_client.h"
#include "od.h"
#include "objects/common.h"

static void _dump_object(lwm2m_object_t *obj);

static void _dump_instance(lwm2m_object_t *obj, lwm2m_list_t *instance);

static void _dump_resource(lwm2m_data_t *data, unsigned level);

void dump_client_objects(lwm2m_client_data_t *client, int obj_id) {
    if (obj_id >= 0) {
        lwm2m_object_t *obj = lwm2m_get_object_by_id(client, obj_id);
        if (!obj) {
            printf("Could not find object with ID: %d\n", obj_id);
            return;
        }
        _dump_object(obj);
    }
    else {
        lwm2m_object_t *obj = client->lwm2m_ctx->objectList;
        if (!obj) {
            printf("The client has no objects\n");
            return;
        }

        while (obj) {
            _dump_object(obj);
            obj = obj->next;
        }
    }
}

static void _dump_object(lwm2m_object_t *obj) {
    assert(obj);

    printf ("├─── Object [/%" PRIu16 "]\n", obj->objID);

    if (!obj->instanceList) {
        printf("│   ├─── No instances yet\n");
        printf("│\n");
        return;
    }

    lwm2m_list_t *instance = obj->instanceList;
    do {
        _dump_instance(obj, instance);
        instance = instance->next;
    } while (instance);

    printf("│\n");
}

static void _dump_instance(lwm2m_object_t *obj, lwm2m_list_t *instance) {
    assert(obj);
    assert(instance);

    int numData = 0;
    lwm2m_data_t *data_array;

    printf("│   ├─── Instance [/%" PRIu16 "/%" PRIu16 "]\n", obj->objID, instance->id);

    obj->readFunc(instance->id, &numData, &data_array, obj);

    for (int i = 0; i < numData; i++) {
        _dump_resource(&data_array[i], 1);
    }

    lwm2m_data_free(numData, data_array);

    printf("│   │\n");
}

static void _dump_resource(lwm2m_data_t *data, unsigned level) {
    assert(data);

    printf("│   ");
    for (unsigned i = 0; i < level; i++) {
        printf("│   ");
    }

    printf("├─── %d : ", data->id);
    
    switch (data->type) {
    case LWM2M_TYPE_STRING:
        printf("%.*s\n", data->value.asBuffer.length, data->value.asBuffer.buffer);
        break;

    case LWM2M_TYPE_OPAQUE:
        if (!data->value.asBuffer.length) {
            printf("<empty buffer>\n");
        }
        else {
            od_hex_dump(data->value.asBuffer.buffer, data->value.asBuffer.length, 0);
        }
        break;

    case LWM2M_TYPE_BOOLEAN:
        printf("%s\n", data->value.asBoolean ? "True" : "False");
        break;

    case LWM2M_TYPE_INTEGER:
        printf("%" PRIi32 "\n", (int32_t)data->value.asInteger);
        break;

    case LWM2M_TYPE_MULTIPLE_RESOURCE:
        for (unsigned i = 0; i < data->value.asChildren.count; i++) {
            printf("<multi-instance resource>\n");
            _dump_resource(&data->value.asChildren.array[i], level+1);
        }
        break;

    default:
        printf("<unknown type (%d)>\n", data->type);
    }
}
