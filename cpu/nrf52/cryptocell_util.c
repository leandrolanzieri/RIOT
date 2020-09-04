#include <stdio.h>
#include "vendor/nrf52840.h"
#include "cryptocell_incl/sns_silib.h"

void cryptocell_enable(void)
{
    NRF_CRYPTOCELL->ENABLE = 1;
    NVIC_EnableIRQ(CRYPTOCELL_IRQn);
}

void cryptocell_disable(void)
{
    NRF_CRYPTOCELL->ENABLE = 0;
    NVIC_DisableIRQ(CRYPTOCELL_IRQn);
}
