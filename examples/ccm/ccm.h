#ifndef CCM_H
#define CCM_H

#include "stdint.h"

typedef enum {
    MILK_CREAM,
    MILK_HNH,
    MILK_WHOLE,
    MILK_P_SKIM,
    MILK_SKIM,
    MILK_NON_DAIRY,
    MILK_NUMOF
} ccm_menu_milk_t;

typedef enum {
    SYRUP_VAN,
    SYRUP_ALM,
    SYRUP_RAS,
    SYRUP_CHO,
    SYRUP_NUMOF
} ccm_menu_syrup_t;

typedef enum {
    ALCOHOL_WHI,
    ALCOHOL_RUM,
    ALCOHOL_KAH,
    ALCOHOL_AQU,
    ALCOHOL_NUMOF
} ccm_menu_alcohol_t;

typedef enum {
    ORDER_STATUS_FREE,
    ORDER_STATUS_WAIT,
    ORDER_STATUS_READY
} ccm_order_status_t;

typedef struct {
    ccm_menu_milk_t milk;
    ccm_menu_syrup_t syrup;
    ccm_menu_alcohol_t alcohol;
} ccm_order_t;

typedef struct {
    ccm_order_t order;
    ccm_order_status_t status;
} ccm_queue_item_t;

typedef struct {
    unsigned count;
    ccm_queue_item_t *queue;
    unsigned queue_len;
} ccm_t;

void ccm_init(ccm_t *dev, ccm_queue_item_t *queue, unsigned queue_len);

static inline unsigned ccm_queue_get_count(ccm_t *dev)
{
    return dev->count;
}

#endif /* CCM_H */