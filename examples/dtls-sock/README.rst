DTLS sock example
#################

This example shows how to use DTLS sock ``sock_dtls_t``.

Testing using RIOT ``native``
---------------------------------

For testing, we can use two RIOT ``native`` RIOT instances. For that first we
need to prepare the network interfaces:

.. code-block:: bash

   $ ./../../dist/tools/tapsetup/tapsetup --create 2

For the server instance:

.. code-block::

   $ PORT=tap0 make all term
   [...]
   > dtlss start
   ifconfig

For the client:

.. code-block::

   $ PORT=tap1 make all term
   [...]
   > dtlsc <server ip address> "DATA to send"

Debug logs
----------

To enable debug logs uncomment ``CFLAGS += -DDTLS_DEBUG`` in the Makefile.
Tinydtls supports setting the log level. See Makefile for more info.

Configs and constraints
-----------------------

DTLS sock acts as a wrapper for the underlying DTLS stack and as such, the
constraints that applies specifically to the stack are also applied here.
For tinydtls, please refer to `dtls-echo README <https://github.com/RIOT-OS/RIOT/blob/master/examples/dtls-echo/README.md>`_.
