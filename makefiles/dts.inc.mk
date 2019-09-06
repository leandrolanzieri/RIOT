export DTB=$(BINDIR)/board.dtb
export BOARD_YML=$(RIOTBOARD)/$(BOARD)/board.yml
export BOARD_SVG=$(RIOTBOARD)/$(BOARD)/board.svg

dbt-prepare:
	gcc -E -P -I$(RIOTBASE)/hwd  -x assembler-with-cpp -o $(BINDIR)/board.dts $(RIOTBOARD)/$(BOARD)/board.dts
	dtc -o $(DTB) $(BINDIR)/board.dts

board-doc: dbt-prepare
	PYTHONPATH=$(RIOTBASE) python $(RIOTTOOLS)/dts/dts_iterator.py

board-pinout: board-doc
