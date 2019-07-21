#ifndef CCM_INTERNAL_H
#define CCM_INTERNAL_H

#include "ccm.h"

#define _STR_SIZE(s)   sizeof(s) - 1

/* Types of milks */
#define MILK_CREAM_STR  "cream"
#define MILK_CREAM_STR_S _STR_SIZE(MILK_CREAM_STR)

#define MILK_HNH_STR  "half-and-half"
#define MILK_HNH_STR_S _STR_SIZE(MILK_HNH_STR)

#define MILK_WHOLE_STR  "whole"
#define MILK_WHOLE_STR_S _STR_SIZE(MILK_WHOLE_STR)

#define MILK_P_SKIM_STR  "part-skim"
#define MILK_P_SKIM_STR_S _STR_SIZE(MILK_P_SKIM_STR)

#define MILK_SKIM_STR  "skim"
#define MILK_SKIM_STR_S _STR_SIZE(MILK_SKIM_STR)

#define MILK_NON_DAIRY_STR  "non-dairy"
#define MILK_NON_DAIRY_STR_S _STR_SIZE(MILK_NON_DAIRY_STR)


/* Types of syrups */
#define SYRUP_VAN_STR   "vanilla"
#define SYRUP_VAN_STR_S _STR_SIZE(SYRUP_VAN_STR)

#define SYRUP_ALM_STR   "almond"
#define SYRUP_ALM_STR_S _STR_SIZE(SYRUP_ALM_STR)

#define SYRUP_RAS_STR   "raspberry"
#define SYRUP_RAS_STR_S _STR_SIZE(SYRUP_RAS_STR)

#define SYRUP_CHO_STR   "chocolate"
#define SYRUP_CHO_STR_S _STR_SIZE(SYRUP_CHO_STR)


/* Types of alcohols */
#define ALCOHOL_WHI_STR "whisky"
#define ALCOHOL_WHI_STR_S _STR_SIZE(ALCOHOL_WHI_STR)

#define ALCOHOL_RUM_STR "rum"
#define ALCOHOL_RUM_STR_S _STR_SIZE(ALCOHOL_RUM_STR)

#define ALCOHOL_KAH_STR "kahlua"
#define ALCOHOL_KAH_STR_S _STR_SIZE(ALCOHOL_KAH_STR)

#define ALCOHOL_AQU_STR "aquavit"
#define ALCOHOL_AQU_STR_S _STR_SIZE(ALCOHOL_AQU_STR)

static const char *_menu_milk_str[] = {
    [MILK_CREAM]     = MILK_CREAM_STR,
    [MILK_HNH]       = MILK_HNH_STR,
    [MILK_WHOLE]     = MILK_WHOLE_STR,
    [MILK_P_SKIM]    = MILK_P_SKIM_STR,
    [MILK_SKIM]      = MILK_SKIM_STR,
    [MILK_NON_DAIRY] = MILK_NON_DAIRY_STR
};

static const unsigned _menu_milk_str_size[] = {
    [MILK_CREAM]     = MILK_CREAM_STR_S,
    [MILK_HNH]       = MILK_HNH_STR_S,
    [MILK_WHOLE]     = MILK_WHOLE_STR_S,
    [MILK_P_SKIM]    = MILK_P_SKIM_STR_S,
    [MILK_SKIM]      = MILK_SKIM_STR_S,
    [MILK_NON_DAIRY] = MILK_NON_DAIRY_STR_S
};

static const char *_menu_syrup_str[] = {
    [SYRUP_VAN] = SYRUP_VAN_STR,
    [SYRUP_ALM] = SYRUP_ALM_STR,
    [SYRUP_RAS] = SYRUP_RAS_STR,
    [SYRUP_CHO] = SYRUP_CHO_STR
};

static const unsigned _menu_syrup_str_size[] = {
    [SYRUP_VAN] = SYRUP_VAN_STR_S,
    [SYRUP_ALM] = SYRUP_ALM_STR_S,
    [SYRUP_RAS] = SYRUP_RAS_STR_S,
    [SYRUP_CHO] = SYRUP_CHO_STR_S
};

static const char *_menu_alcohol_str[] = {
    [ALCOHOL_WHI] = ALCOHOL_WHI_STR,
    [ALCOHOL_RUM] = ALCOHOL_RUM_STR,
    [ALCOHOL_KAH] = ALCOHOL_KAH_STR,
    [ALCOHOL_AQU] = ALCOHOL_AQU_STR
};

static const unsigned _menu_alcohol_str_size[] = {
    [ALCOHOL_WHI] = ALCOHOL_WHI_STR_S,
    [ALCOHOL_RUM] = ALCOHOL_RUM_STR_S,
    [ALCOHOL_KAH] = ALCOHOL_KAH_STR_S,
    [ALCOHOL_AQU] = ALCOHOL_AQU_STR_S
};

#endif /* CCM_INTERNAL_H */
