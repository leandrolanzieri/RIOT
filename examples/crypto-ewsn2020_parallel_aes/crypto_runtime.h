#ifndef CRYPTO_RUNTIME_H
#define CRYPTO_RUNTIME_H

#include "periph/gpio.h"

void aes_ecb_parallel_test(gpio_t *thread1_pin, gpio_t *thread2_pin);

/* GPIOs used to trace encyprion times */
gpio_t thread1_pin = GPIO_PIN(2, 9);
gpio_t thread2_pin = GPIO_PIN(3, 9);

#endif /* CRYPTO_RUNTIME_H */
