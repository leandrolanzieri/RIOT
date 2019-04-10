APPLICATION = test_registry_cbor
include ../Makefile.tests_common

USEMODULE += shell
USEMODULE += registry
USEMODULE += memarray

CFLAGS += -DTHREAD_STACKSIZE_MAIN=\(2*THREAD_STACKSIZE_DEFAULT\)

include $(RIOTBASE)/Makefile.include
