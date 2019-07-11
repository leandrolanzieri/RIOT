$(MENUCONFIG):
	@echo "[INFO] Kconfiglib not found - getting it"
	@make -C $(RIOTTOOLS)/kconfiglib
	@echo "[INFO] Kconfiglib downloaded"

$(GENCONFIG): $(MENUCONFIG)
