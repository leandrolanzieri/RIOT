RIOT Documentation
==================
This is the unofficial documentation for RIOT.

.. toctree::
   :maxdepth: 1
   :glob:

   examples.rst

RIOT in a nutshell
==================
RIOT is an open-source  microkernel-based operating system, designed to match
the requirements of Internet of Things (IoT) devices and other embedded
devices. These requirements include a very low memory footprint (on the order
of a few kilobytes), high energy efficiency, real-time capabilities, support
for a wide range of low-power hardware, communication stacks for wireless and
communication stacks for wired networks.

RIOT provides a microkernel, multiple network stacks, and utilities which
include cryptographic libraries, data structures (bloom filters, hash tables,
priority queues), a shell and more. RIOT supports a wide range of
microcontroller architectures, radio drivers, sensors, and configurations for
entire platforms, e.g. Atmel SAM R21 Xplained Pro, Zolertia Z1, STM32 Discovery
Boards etc. (see the list of
[supported hardware](https://github.com/RIOT-OS/RIOT/wiki/RIOT-Platforms).
Across all supported hardware (32-bit, 16-bit, and 8-bit platforms). RIOT
provides a consistent API and enables ANSI C and C++ application programming,
with  multithreading, IPC, system timers, mutexes etc.

A good high-level overview can be found in the article
[RIOT: An Open Source Operating System for Low-End Embedded Devices in
the IoT](https://riot-os.org/docs/riot-ieeeiotjournal-2018.pdf)
(IEEE Internet of Things Journal, December 2018).

