## Usage

This application can be used to measure processing time, energy consumption, stack size and memory usage of the ECDH operation using the relic library.

It runs on the NRF52840DK.

Per default relic uses its internal SHA256-PRNG.
To use RIOT HWRNG, build with RIOT_HWRNG=1.

#### Processing Time
- Connect a logic analyzer to the GPIOs defined in main.c
- *make BOARD=nrf52850dk*

#### Energy Consumption
- Set TEST_ENERGY=1 when building the application
- *TEST_ENERGY=1 make BOARD=nrf52850dk*
# PETER please add info

#### Stack Test
Builds a minimal application to test size of used stack
Prints the output of PS in terminal
- Set TEST_STACK=1 when building the app

#### Memory Test
Builds minimal application to later analyze binary files for memory usage
- Set TEST_MEM=1 when building the app