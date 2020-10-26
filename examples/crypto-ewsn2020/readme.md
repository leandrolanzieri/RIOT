### Usage

This application can be used to measure the processing time, energy consumption and memory usage of sha256 and hmac-sha256 calculation as well as aes-128-ecb and aes-128-cbc encryption and decryption.

#### Boards:
- pba-d-01-kw2x
- slstk3402a
- nrf52840dk (Optionally with Microchip ATECC608A extension)

#### Build variants
##### Processing Time
- Connect logic analyzer to gpios defined in crypto_runtime.h
- Enable desired algorithm in menuconfig and chose hardware or software implementation
    - e.g.: *make BOARD=xxx menuconfig*
    - If using the pba-d-01-kw2x board and AES ECB or CBC, general AES must be hardware and mode specific implementations must be software
    - For all other boards both general AES and mode specific implementations must be hardware
- If using nrf52840dk with ATECC608A extension
    - Enable Microchip CryptoAuth Package in menuconfig
    - Configure desired algorithm to use CryptoAuth implementation
    - I2C Reconfigure to change I2C bus speed is enabled per default in Kconfig file
    - Optionally enable manual calls of wake, idle and sleep when building app
        - ATCA_MANUAL_ONOFF=1
- Set desired algorithm
    - Options:
        - SHA256
        - HMAC (32 Byte HMAC Key)
        - HMAC_64 (64 Byte HMAC Key – not compatible with ATECC608A)
        - AES_ECB
        - AES_CBC
- Optionally set input size of 512 Byte (default is 32 Byte)
    - e.g.: *SHA256=1 INPUT_512=1 make BOARD=xxx*

##### Energy Consumption

# PETER please add info

- Enable desired algorithm in menuconfig as described above
- Set desired algorithm when building the application
    - Options:
        - TEST_ENERGY_SHA256
        - TEST_ENERGY_HMAC_SHA256
        - TEST_ENERGY_AES_ECB_<ENC|DEC>
        - TEST_ENERGY_AES_CBC_<ENC|DEC>
- Optionally set desired number of iterations (default is 1100)
    - e.g.: *TEST_ENERGY_SHA256=1 TEST_ENERGY_ITER=500 make BOARD=xxx*
- Optionally use xtimer to delay to start measurement tool
    - USE_XTIMER=1

##### Stack Size
- Builds minimal application to measure size of used stack
    - PS output will be printed at the end of the program
- Enable desired algorithm in menuconfig as described above
- Enable PS support in menuconfig
- Disable xtimer in menuconfig
- Set desired algorithm and TEST_STACK when building app
    - e.g.: *TEST_STACK=1 TEST_ENERGY_SHA256=1 make BOARD=xxx*

##### Memory Test
- Builds minimal application for later analysis of binaries
- Enable desired algorithm in menuconfig as described above
- Disable all unneeded modules in menu config (e.g. xtimer, other crypto algorithms, PS support, …)
- Set desired algorithms and TEST_MEM when building app
    - e.g.: *TEST_MEM=1 TEST_ENERGY_SHA256=1 make BOARD=xxx*

# PETER how about the software tests? How did you include the other packages?