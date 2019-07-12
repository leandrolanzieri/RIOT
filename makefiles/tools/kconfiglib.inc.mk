# Define tools to use
MENUCONFIG = $(RIOTTOOLS)/kconfiglib/menuconfig.py
GENCONFIG = $(RIOTTOOLS)/kconfiglib/genconfig.py

$(MENUCONFIG):
	@echo "[INFO] Kconfiglib not found - getting it"
	@make -C $(RIOTTOOLS)/kconfiglib
	@echo "[INFO] Kconfiglib downloaded"

$(GENCONFIG): $(MENUCONFIG)
