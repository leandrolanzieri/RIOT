#ifndef HWCRYPTO_H
#define HWCRYPTO_H

#include "periph_cpu.h"
#include "periph_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Default hardware crypto device access macro
 */
#ifndef HWCRYPTO_DEV
#define HWCRYPTO_DEV(x)     (x)
#endif

/**
 * @brief   Hardware crypto type identifier
 */
#ifndef HAVE_HWCRYPTO_T
typedef unsigned int hwcrypto_t;
#endif

/**
 * @brief   Initialize the hardware crypto peripheral.
 *
 * The method should initialize the peripheral. It may power on the device if
 * needed, but it must ensure that the peripheral is powered off when exiting
 * this method.
 *
 * This function is intended to be called by the board initialization code
 * during system startup to prepare the (shared) hardware crypto device for
 * further usage. It uses the board specific initialization parameters as
 * defined in the board's `periph_conf.h`.
 *
 * Errors (e.g. invalid @p dev parameter) are not signaled through a return
 * value, but should be signaled using the assert() function internally.
 *
 * @param[in] dev           the device to initialize
 */
void hwcrypto_init(hwcrypto_t dev);

#endif /* HWCRYPTO_H */