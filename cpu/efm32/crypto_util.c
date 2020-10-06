#include <stdbool.h>
#include <stdio.h>
#include "crypto_util.h"
#include "mutex.h"
#include "xtimer.h"

#include "periph/gpio.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#if CPU_MODEL_EFM32PG12B500F1024GL125
static int acqu_count = 0;
static bool crypto_lock_initialized = false;
static crypto_device_t crypto_devs[CRYPTO_COUNT] =
{
    #if defined(CRYPTO0)
        {
            CRYPTO0,
            cmuClock_CRYPTO0,
            CRYPTO0_IRQn,
            MUTEX_INIT,
            MUTEX_INIT,
            GPIO_PIN(0, 6),
            1,
            0
        },
    #elif defined(CRYPTO)
        {
            CRYPTO,
            cmuClock_CRYPTO,
            CRYPTO_IRQn,
            MUTEX_INIT,
            MUTEX_INIT,
            GPIO_UNDEF,
            3,
            2
        },
    #endif
    #if defined(CRYPTO1)
        {
            CRYPTO1,
            cmuClock_CRYPTO1,
            CRYPTO1_IRQn,
            MUTEX_INIT,
            MUTEX_INIT,
            GPIO_PIN(0, 7),
            3,
            2
        }
    #endif
};

CRYPTO_TypeDef* crypto_acquire(void) {
    CRYPTO_TypeDef* dev = NULL;
    if (!crypto_lock_initialized) {
        /* initialize lock */
        for (int i = 0; i < CRYPTO_COUNT; i++) {
            mutex_init(&crypto_devs[i].lock);
            mutex_init(&crypto_devs[i].sequence_lock);
            mutex_lock(&crypto_devs[i].sequence_lock);
            NVIC_ClearPendingIRQ(crypto_devs[i].irq);
            NVIC_EnableIRQ(crypto_devs[i].irq);
        }
        crypto_lock_initialized = true;
    }

    int devno = acqu_count % CRYPTO_COUNT;
    mutex_lock(&crypto_devs[devno].lock);
    dev = crypto_devs[devno].dev;

    gpio_set(crypto_devs[devno].pin);

    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(crypto_devs[devno].cmu, true);

    acqu_count++;
    return dev;
}

crypto_device_t* crypto_acquire_dev(void) {
    crypto_device_t* dev = NULL;
    if (!crypto_lock_initialized) {
        /* initialize lock */
        for (int i = 0; i < CRYPTO_COUNT; i++) {
            mutex_init(&crypto_devs[i].lock);
            mutex_init(&crypto_devs[i].sequence_lock);
            mutex_lock(&crypto_devs[i].sequence_lock);
            NVIC_ClearPendingIRQ(crypto_devs[i].irq);
            NVIC_EnableIRQ(crypto_devs[i].irq);
        }
        crypto_lock_initialized = true;
    }

    int devno = acqu_count % CRYPTO_COUNT;
    mutex_lock(&crypto_devs[devno].lock);
    dev = &crypto_devs[devno];

    gpio_set(crypto_devs[devno].pin);

    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(crypto_devs[devno].cmu, true);

    acqu_count++;
    return dev;
}

static int get_devno(CRYPTO_TypeDef* dev)
{
    if (CRYPTO_COUNT == 1) {
        return 0;
    }
    else if (CRYPTO_COUNT == 2 && dev == CRYPTO0) {
        return 0;
    }
    else if (CRYPTO_COUNT == 2 && dev == CRYPTO1) {
        return 1;
    }
    return -1;
}

crypto_device_t* crypto_get_dev_by_crypto(CRYPTO_TypeDef* crypto)
{
    int devno = get_devno(crypto);
    return &crypto_devs[devno];
}

void crypto_release(CRYPTO_TypeDef* dev)
{
    int devno = get_devno(dev);
    if (devno < 0) {
        return;
    }
    CMU_ClockEnable(crypto_devs[devno].cmu, false);

    gpio_clear(crypto_devs[devno].pin);
    acqu_count--;
    mutex_unlock(&crypto_devs[devno].lock);
}

void crypto_wait_for_sequence(CRYPTO_TypeDef *dev)
{
    /* enable interrupt on sequence done */
    CRYPTO_IntEnable(dev, CRYPTO_IEN_SEQDONE);

    /* get the sequence lock */
    int devno = get_devno(dev);
    if (devno < 0) {
        return;
    }

    mutex_t *lock = &(crypto_devs[devno].sequence_lock);

    /* start the sequence */
    dev->CMD = CRYPTO_CMD_SEQSTART;

    /* wait for the sequence to finish */
    mutex_lock(lock);
    //xtimer_usleep(100);

    /* disable interrupt on sequence done */
    CRYPTO_IntDisable(dev, CRYPTO_IEN_SEQDONE);
}


// static void crypto_irq(CRYPTO_TypeDef* dev) {
//     int devno = get_devno(dev);
//     CRYPTO_IntClear(dev, CRYPTO_IFC_SEQDONE | CRYPTO_IFC_INSTRDONE);
//     mutex_unlock(&(crypto_devs[devno].sequence_lock));
// }

#endif

// void isr_crypto0(void) {
//     crypto_irq(CRYPTO0);
// }


// void isr_crypto1(void) {
//     crypto_irq(CRYPTO1);
// }

// void isr_crypto(void) {
//     crypto_irq(CRYPTO);
// }
