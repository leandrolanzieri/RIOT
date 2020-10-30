## Config and Lock ATECC608A Config & Data Zone
This application can be used to configure the Microchip ATECC608A's config zone and lock both config and data zones.

The config pattern is declared as an array in main.c. Most key slots are configured for the use of ECC functions. Only a few were left for other operations, but not all of them were tested.

For further details about slot configuration please refer to the according sections in the reference manual.

#### Usage

Configuring and locking are separate and conditional. If none of the conditionals is set, the application will just read and display the config zone and the lock status in the terminal.

- config: build the application with CONFIG_CRYPTO=1
- lock (locks config AND data zone): build with LOCK_CRYPTO=1
    - if zones are already locked, app will display a message that locking went wrong in terminal

After running the above operations the application will read and display the config zone and confirm the lock status in the terminal.