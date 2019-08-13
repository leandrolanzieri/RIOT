CC_PREFIX = 
CC = $(CC_PREFIX)gcc
OUT ?= $(BOARD).dtb

all:
	$(CC) -E -P -x assembler-with-cpp $(BOARD).dts | dtc -o $(OUT)
