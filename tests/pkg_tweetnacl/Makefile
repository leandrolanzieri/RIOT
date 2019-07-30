include ../Makefile.tests_common

BOARD_INSUFFICIENT_MEMORY := arduino-duemilanove \
                             arduino-leonardo \
                             arduino-mega2560 \
                             arduino-nano \
                             arduino-uno \
                             mega-xplained \
                             nucleo-f031k6 \
                             nucleo-f042k6 \
                             #

TEST_ON_CI_WHITELIST += native

CFLAGS += -DCONFIG_THREAD_STACKSIZE_MAIN=\(5*CONFIG_THREAD_STACKSIZE_DEFAULT\)

USEPKG += tweetnacl
USEMODULE += embunit

include $(RIOTBASE)/Makefile.include
