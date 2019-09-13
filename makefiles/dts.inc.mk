# Tools
# Device Tree Compiler
DTC ?= dtc

# Dependencies
# board device tree
DTS ?= $(RIOTBOARD)/$(BOARD)/board.dts
HWD_INCLUDE = $(RIOTHWD)/include
BOARD_YML=$(RIOTBOARD)/$(BOARD)/board.yml
export TEMPLATE_PINOUT ?= boards/$(BOARD)/board.svg
export DTS_TEMPLATES = $(RIOTTOOLS)/dts/templates
# Artifacts
# board device tree blob
DTB = $(BINDIR)/board.dtb
# flatten device tree
FDTS = $(BINDIR)/board.dts

RIOTHWD = $(RIOTBASE)/hwd

dtb-prepare:
	$(Q)$(CC) -E -P -I$(HWD_INCLUDE) -I$(RIOTHWD) -x assembler-with-cpp -o $(FDTS) $(DTS)
	$(Q)$(DTC) -W no-unit_address_vs_reg -o $(DTB) $(FDTS)

board-pinout: dtb-prepare
	$(Q)PYTHONPATH=$(RIOTBASE) python3 \
		$(RIOTTOOLS)/dts/dts_iterator.py $(DTB) pinout -b $(BOARD_YML) -p $(TEMPLATE_PINOUT) --log=INFO

gen-periph-conf: dtb-prepare
	$(Q)PYTHONPATH=$(RIOTBASE) python3 \
		$(RIOTTOOLS)/dts/dts_iterator.py $(DTB) generate -b $(BOARD_YML) --log=INFO