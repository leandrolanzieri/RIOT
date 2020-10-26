## Usage

This application can be used to measure processing time, energy consumption, stack size and memory usage of the hardware accelerated ECDSA operation using the CryptoCell module on the NRF52840DK and the cryptocell library.

#### Processing Time
To measure processing time, connect a logic analyzer to the GPIOs defined in main.c and build the application
- *make BOARD=nrf52850dk*

#### Energy Consumption
- Set TEST_ENERGY=1 when building the application
- *TEST_ENERGY=1 make BOARD=nrf52850dk*
# PETER please add info

#### Stack Test
Builds a minimal application to test size of used stack
Prints PS Output in terminal
- Set TEST_STACK=1 when building the app

#### Memory Test
Builds minimal application to later analyze binary files for memory usage
- Set TEST_MEM=1 when building the app