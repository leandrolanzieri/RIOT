## Usage

This application can be used to measure processing time, energy consumption, stack size and memory usage of the hardware accelerated ECDH operation using the CryptoAuth ATECC608A device and the CryptoAuth Lib.

This application is only executable, if the ATECC608A is correctly configured and both config and data zone are locked. An application to configure and lock the device with the exact same configuration we used can be found here:

https://github.com/Einhornhool/RIOT/tree/cryptoauthlib-extract-wake-sleep/examples/crypto-ewsn2020-cryptoauth-config-lock

#### Equipment
This application runs on a Nordic NRF52840DK with a ATECC608A extension

#### Processing Time
To measure processing time, connect a logic analyzer to the GPIOs defined in main.c and build the application
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
- Set TEST_MEM=1 when building the app
