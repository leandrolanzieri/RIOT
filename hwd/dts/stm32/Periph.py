from lwd import *
from peewee import *
import peewee

class Usart(BaseModel):
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

class UsartPinmux(BasePinmux):
    periph = Periph(Usart, backref='pinmux')
    tx_pin = Pin()
    rx_pin = Pin()
    tx_af = AlternateFunction()
    rx_af = AlternateFunction()

    def get_label(self):
        return "{}_tx_{}_{}_rx_{}_{}".format(
                self.periph,
                self.tx_pin.port, self.tx_pin.num,
                self.rx_pin.port, self.rx_pin.num).lower()

class Spi(BaseModel):
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

class SpiPinmux(BasePinmux):
    periph = ForeignKeyField(Spi)
    mosi_pin = Pin()
    miso_pin = Pin()
    af = CharField()

    def get_label(self):
        return "{}_mosi_{}_{}_miso_{}_{}".format(
                self.periph,
                self.mosi_pin.port, self.mosi_pin.num,
                self.miso_pin.port, self.miso_pin.num).lower()

class I2C(BaseModel):
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

class I2CPinmux(BasePinmux):
    periph = ForeignKeyField(I2C)
    scl_pin = Pin()
    sda_pin = Pin()
    scl_af = CharField()
    sda_af = CharField()

    def get_label(self):
        return "{}_scl_{}_{}_sda_{}_{}".format(
                self.periph,
                self.scl_pin.port, self.scl_pin.num,
                self.sda_pin.port, self.sda_pin.num).lower()

class Dma(BaseModel):
    type = CharField()
    device = CharField()

class I2CConfig(BasePinmuxConfig):
    pinmux = ForeignKeyField(I2CPinmux)
    speed = CharField()

class UsartConfig(BasePinmuxConfig):
    pinmux = ForeignKeyField(UsartPinmux)

class SpiConfig(BasePinmuxConfig):
    pinmux = ForeignKeyField(SpiPinmux)
    cs_pin = Pin()

    def get_pins(self):
        l = [{'pin':self.cs_pin, 'function':'cs_pin'}]
        l.extend(self.pinmux.get_pins())
        for p in l:
            p['config_group'] = self.config_group
        return l