#include "ccm.h"
#include "coral.h"

void ccm_init(ccm_t *dev, ccm_queue_item_t *queue, unsigned queue_len)
{
    dev->count = 0;
    dev->queue = queue;
    dev->queue_len = queue_len;

    for (unsigned i = 0; i < queue_len; i++) {
        dev->queue[i].status = ORDER_STATUS_FREE;
    }

    coral_init();
}
