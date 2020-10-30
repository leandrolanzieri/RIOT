## Usage

This app can be used to test the sha256 function of the ATECC608A when using two different contexts and storing them externally between calls.

This app has only been tested using the NRF52840DK with an ATECC608A Extension.

It's possible to turn off I2C Reconfigure by building with
NO_I2C_RECONF=1