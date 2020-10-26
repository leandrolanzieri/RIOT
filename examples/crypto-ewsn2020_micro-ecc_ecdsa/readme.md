## Usage

This application can be used to measure processing time, energy consumption, stack size and memory usage of the ECDSA operation using the uECC library.

It runs on the NRF52840DK.

There are two RNGs available: SHA256-PRNG (Default) and RIOT HWRNG. They can be configured in menuconfig:
- *make BOARD=nrf52840dk menuconfig*

#### Processing Time
- Connect a logic analyzer to the GPIOs defined in main.c
- *make BOARD=nrf52850dk*

#### Energy Consumption
- Set TEST_ENERGY=1 when building the application
- *TEST_ENERGY=1 make BOARD=nrf52850dk*
# PETER please add info

#### Stack Test
Builds a minimal application to test size of used stack
Prints PS output in terminal
- Enable PS Support in menuconfig
- Set TEST_STACK=1 when building the app

#### Memory Test
Builds minimal application to later analyze binary files for memory usage
- Make sure no unnecessary modules are enabled in menuconfig (e.g. PS Support)
- Set TEST_MEM=1 when building the app