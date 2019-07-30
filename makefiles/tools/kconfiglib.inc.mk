# Define tools to use
MENUCONFIG ?= $(RIOTTOOLS)/kconfiglib/menuconfig.py
GENCONFIG ?= $(RIOTTOOLS)/kconfiglib/genconfig.py
GUICONFIG ?= $(RIOTTOOLS)/kconfiglib/guiconfig.py
MERGECONFIG ?= $(RIOTTOOLS)/kconfiglib/merge_config.py

$(MENUCONFIG):
	@echo "[INFO] Kconfiglib not found - getting it"
	@make -C $(RIOTTOOLS)/kconfiglib
	@echo "[INFO] Kconfiglib downloaded"

$(GENCONFIG): $(MENUCONFIG)

$(GUICONFIG): $(MENUCONFIG)

$(MERGECONFIG): $(MENUCONFIG)
