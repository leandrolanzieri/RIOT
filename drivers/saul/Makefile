SRC = saul.c saul_str.c

ifneq (,$(filter saul_gpio,$(USEMODULE)))
  SRC += gpio_saul.c
endif
ifneq (,$(filter saul_adc,$(USEMODULE)))
  SRC += adc_saul.c
endif
ifneq (,$(filter saul_rgb_led, $(USEMODULE)))
  SRC += rgb_led_saul.c
endif

include $(RIOTBASE)/Makefile.base
