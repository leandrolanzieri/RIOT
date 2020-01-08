GCOAP
#####

.. role:: raw-html-m2r(raw)
   :format: html


About
-----

This application provides command line access to gcoap, a high-level API for
CoAP messaging. See the :raw-html-m2r:`<a href="https://tools.ietf.org/html/rfc7252" title="CoAP spec">CoAP spec</a>` for background, and the
Modules>Networking>CoAP topic in the source documentation for detailed usage
instructions and implementation notes.

We support two setup options for this example:

Native networking
^^^^^^^^^^^^^^^^^

Build with the standard ``Makefile``. Follow the setup :raw-html-m2r:`<a href="https://github.com/RIOT-OS/RIOT/tree/master/examples/gnrc_networking" title="instructions">instructions</a>` for
the gnrc_networking example.

SLIP-based border router
^^^^^^^^^^^^^^^^^^^^^^^^

Build with ``Makefile.slip``. Follow the setup instructions in README-slip.md,
which are based on the :raw-html-m2r:`<a href="https://github.com/RIOT-OS/RIOT/tree/master/examples/gnrc_border_router" title="SLIP instructions">SLIP instructions</a>` for the gnrc_border_router
example. We also plan to provide or reference the ethos/UHCP instructions,
but we don't have it working yet.

Example Use
-----------

This example uses gcoap as a server on RIOT native. Then we send a request
from a libcoap example client on the Linux host.

Verify setup from RIOT terminal
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block::

   > coap info


Expected response:

.. code-block::

   CoAP server is listening on port 5683
    CLI requests sent: 0
   CoAP open requests: 0


Query from libcoap example client
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

gcoap does not provide any output to the CoAP terminal when it handles a
request. We recommend use of Wireshark to see the request and response. You
also can add some debug output in the endpoint function callback.

.. code-block::

   ./coap-client -N -m get -p 5683 coap://[fe80::1843:8eff:fe40:4eaa%tap0]/.well-known/core


Example response:

.. code-block::

   v:1 t:NON c:GET i:0daa {} [ ]
   </cli/stats>


The response shows the endpoint registered by the gcoap CLI example.

Send query to libcoap example server
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Start the libcoap example server with the command below.

.. code-block::

   ./coap-server


Enter the query below in the RIOT CLI.

.. code-block::

   > coap get fe80::d8b8:65ff:feee:121b%6 5683 /.well-known/core


CLI output:

.. code-block::

   gcoap_cli: sending msg ID 743, 75 bytes
   > gcoap: response Success, code 2.05, 105 bytes
   </>;title="General Info";ct=0,</time>;if="clock";rt="Ticks";title="Internal Clock";ct=0;obs,</async>;ct=0



Other available CoAP implementations and applications
-----------------------------------------------------

RIOT also provides package imports and test applications for other CoAP
implementations:


* 
  `Nanocoap <../nanocoap_server>`_\ : a very lightweight CoAP server based on the
  `nanocoap library <https://github.com/kaspar030/sock/tree/master/nanocoap>`_
  implementation

* 
  `Microcoap <../../tests/pkg_microcoap>`_\ : another lightweight CoAP server based
  on the `microcoap library <https://github.com/1248/microcoap>`_ implementation
