# Tools
include $(RIOTMAKE)/tools/dtc.inc.mk
DTS_ITERATOR = $(RIOTBASE)/dist/pythonlibs/dts/dts_iterator.py

# Dependencies
# board device tree
DTS ?= $(RIOTBOARD)/$(BOARD)/board.dts
HWD_INCLUDE = $(RIOTHWD)/include
BOARD_YML=$(RIOTBOARD)/$(BOARD)/board.yml
export TEMPLATE_PINOUT ?= boards/$(BOARD)/board.svg

export DTS_TEMPLATES = $(RIOTBASE)/dist/pythonlibs/dts/templates
RIOTHWD = $(RIOTBASE)/hwd

# Artifacts
# board device tree blob
DTB = $(BINDIR)/board.dtb
# flatten device tree
FDTS = $(BINDIR)/board.dts

dtb-prepare: $(DTC)
	$(Q)$(CC) -E -P -I$(HWD_INCLUDE) -I$(RIOTHWD) -x assembler-with-cpp -o $(FDTS) $(DTS)
	$(Q)$(DTC) -W no-unit_address_vs_reg -o $(DTB) $(FDTS)

board-pinout: dtb-prepare
	$(Q)PYTHONPATH=$(RIOTBASE) python3 \
		$(DTS_ITERATOR) $(DTB) pinout -b $(BOARD_YML) -p $(TEMPLATE_PINOUT) --log=INFO

print-pinout: dtb-prepare
	$(Q)PYTHONPATH=$(RIOTBASE) python3 \
		$(DTS_ITERATOR) $(DTB) pinout -b $(BOARD_YML) --log=INFO

gen-periph-conf: dtb-prepare
	$(Q)PYTHONPATH=$(RIOTBASE) python3 \
		$(DTS_ITERATOR) $(DTB) generate -b $(BOARD_YML) --log=INFO
