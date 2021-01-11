## Parallel AES-ECB encryption

This application shows how to use the AES hardware acceleration in the
SLSTK3402A board (featuring a Silabs EFM32PG12B500F1024GL125 microcontroller)
from multiple RIOT threads to achieve true parallelism. The application also
serves as an evaluation on the effects parallelism has on speed and CPU
availability.

The application initiates two threads that encrypt an array of bytes, using the
ECB cipher mode. The length of this array is configurable (see Configuration
section). This array is encrypted 1000 times. The implementation of the
`aes_encrypt_ecb` function can be configured to utilize multiple instances of
the acceleration peripheral together with DMA channels to free the CPU while the
operation is performed. This allows the scheduler to switch to a different
thread. Up to two simultaneous operations can be parallelized.


### Configuration

All configurations are accessible via the menuconfig interface. Run
`make menuconfig` to open it.

#### Parallel encryption
To enable the parallel AES ECB encryption, set `EFM32_AES_ECB_NONBLOCKING` to
`y` in the configuration interface.

#### 512-bits encryption block
The length of the array to encrypt can be switched to 512 bytes by setting
`INPUT_512` to `y`. If not set, the length of the array 32 bytes long.


### Trace GPIOs

The following GPIO pins are configured in the SLSTK3402A board, to trace the
time that the AES encryption operations take:

- **thread1_pin** (PC9): Thread 1 controls this pin. Set to HIGH before each
  encryption of the array. Set to LOW after the each encryption of the array.
  You should see 1000 pulses (one per iteration).
- **thread2_pin** (PD9): Thread 2 controls this pin. Set to HIGH before each
  encryption of the array. Set to LOW after the each encryption of the array.
  You should see 1000 pulses (one per iteration).
- **crypto0_pin** (PA6): `CRYPTO0` instance of the crypto-accelerator peripheral
  controls this pin. Set to HIGH before the encryption of a block begins. Set to
  LOW after the encryption of a block ends.
- **crypto1_pin** (PA7): `CRYPTO1` instance of the crypto-accelerator peripheral
  controls this pin. Set to HIGH before the encryption of a block begins. Set to
  LOW after the encryption of a block ends.

When parallel encryption is set, it is expected to see activity on both CRYPTO
peripheral instances (`crypto0_pin` and `crypto1_pin`), and an overlap in the
activity of both threads.
