DTC ?= $(RIOTTOOLS)/dtc/dtc

$(DTC):
	@echo "[INFO] Device Tree compiler not found - getting it"
	@make -C $(RIOTTOOLS)/dtc
	@echo "[INFO] Device Tree compiler downloaded"
