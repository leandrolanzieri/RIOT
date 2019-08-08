from dts import base
from peewee import *

class Usart(base.BaseModel):
    type = CharField()
    device = CharField()
    bus = CharField()
    rcc = CharField()
    irqn = CharField()
    isr = CharField()
    tx_dma = CharField()
    tx_dma_stream = CharField()
    tx_dma_channel = CharField()
    rx_dma = CharField()
    rx_dma_stream = CharField()
    rx_dma_channel = CharField()

class UsartPinmux(base.BasePinmux):
    periph = base.Periph(Usart)
    tx = base.Pin()
    rx = base.Pin()
    tx_af = base.AlternateFunction()
    rx_af = base.AlternateFunction()

class Spi(base.BaseModel):
    type = CharField()
    device = CharField()
    bus = CharField()
    rcc = CharField()
    tx_dma = CharField()
    tx_dma_stream = CharField()
    tx_dma_channel = CharField()
    rx_dma = CharField()
    rx_dma_stream = CharField()
    tx_dma_channel = CharField()

class SpiPinmux(base.BasePinmux):
    periph = ForeignKeyField(Spi)
    mosi = base.Pin()
    miso = base.Pin()
    sck = base.Pin()
    af = CharField()

class I2C(base.BaseModel):
    type = CharField()
    device = CharField()
    bus = CharField()
    rcc = CharField()
    clk = CharField()
    irqn = CharField()
    isr = CharField()
    tx_dma = CharField()
    tx_dma_stream = CharField()
    tx_dma_channel = CharField()
    rx_dma = CharField()
    rx_dma_stream = CharField()
    rx_dma_channel = CharField()

class I2CPinmux(base.BasePinmux):
    periph = ForeignKeyField(I2C)
    scl = base.Pin()
    sda = base.Pin()
    scl_af = CharField()
    sda_af = CharField()

class Dma(base.BaseModel):
    type = CharField()
    device = CharField()

class I2CConfig(base.BasePinmuxConfig):
    pinmux = ForeignKeyField(I2CPinmux)
    speed = CharField()

class UsartConfig(base.BasePinmuxConfig):
    pinmux = ForeignKeyField(UsartPinmux)

class SpiConfig(base.BasePinmuxConfig):
    pinmux = ForeignKeyField(SpiPinmux)
    cs = base.Pin()

    def get_pins(self):
        l = [{'pin':self.cs, 'function':'cs'}]
        l.extend(self.pinmux.get_pins())
        for p in l:
            p['config_group'] = self.config_group
        return l
