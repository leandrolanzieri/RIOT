from dts import base
from dts.base import pinmux
from peewee import *

class Usart(base.BaseModel):
    type = base.Macro()
    device = base.Macro()
    bus = base.Macro()
    rcc = base.Macro()
    irqn = base.Macro()
    isr = base.Macro()
    tx_dma = base.Macro()
    tx_dma_stream = base.Macro()
    tx_dma_channel = base.Macro()
    rx_dma = base.Macro()
    rx_dma_stream = base.Macro()
    rx_dma_channel = base.Macro()

@pinmux(Usart)
class UsartPinmux(base.BasePinmux):
    tx = base.Pin()
    rx = base.Pin()
    tx_af = base.AlternateFunction()
    rx_af = base.AlternateFunction()

class Spi(base.BaseModel):
    type = base.Macro()
    device = base.Macro()
    bus = base.Macro()
    rcc = base.Macro()
    tx_dma = base.Macro()
    tx_dma_stream = base.Macro()
    tx_dma_channel = base.Macro()
    rx_dma = base.Macro()
    rx_dma_stream = base.Macro()
    tx_dma_channel = base.Macro()

@pinmux(Spi)
class SpiPinmux(base.BasePinmux):
    mosi = base.Pin()
    miso = base.Pin()
    #sck = base.Pin()
    af = base.Macro()

class I2C(base.BaseModel):
    type = base.Macro()
    device = base.Macro()
    bus = base.Macro()
    rcc = base.Macro()
    clk = base.Macro()
    irqn = base.Macro()
    isr = base.Macro()
    tx_dma = base.Macro()
    tx_dma_stream = base.Macro()
    tx_dma_channel = base.Macro()
    rx_dma = base.Macro()
    rx_dma_stream = base.Macro()
    rx_dma_channel = base.Macro()

@pinmux(I2C)
class I2CPinmux(base.BasePinmux):
    scl = base.Pin()
    sda = base.Pin()
    scl_af = base.Macro()
    sda_af = base.Macro()

class Dma(base.BaseModel):
    type = base.Macro()
    device = base.Macro()

class I2CConfig(base.BasePinmuxConfig):
    pinmux = base.Pinmux(I2CPinmux)
    speed = base.Macro()

class UsartConfig(base.BasePinmuxConfig):
    pinmux = base.Pinmux(UsartPinmux)

class SpiConfig(base.BasePinmuxConfig):
    pinmux = base.Pinmux(SpiPinmux)
    cs = base.Pin()

