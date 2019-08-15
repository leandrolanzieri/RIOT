from peewee import *
from hwd.dts.common import node, NodeModel, db, BaseModel, Cell
from hwd.dts.common import Pinctrl

@node(cells=['stream', 'channel'])
class Dma(NodeModel):
    device = CharField()
    class Meta:
        database = db

@node(cells=['num', 'flags'])
class Gpio(NodeModel):
    label = CharField()

@node()
class UsartPinctrl(Pinctrl):
    tx = Cell(Gpio.cell_class())
    rx = Cell(Gpio.cell_class())

@node()
class Usart(NodeModel):
    device = CharField()
    rcc = CharField()
    interrupts = CharField()
    isr = CharField()
    status = CharField()
    tx_dma = Cell(Dma.cell_class(), column_name="tx-dma")
    rx_dma = Cell(Dma.cell_class(), column_name="rx-dma")
    pinctrl = Cell(UsartPinctrl.cell_class(), null=True)

@node()
class SpiPinctrl(Pinctrl):
    miso = Cell(Gpio.cell_class())
    mosi = Cell(Gpio.cell_class())
    sck = Cell(Gpio.cell_class())
    cs = Cell(Gpio.cell_class())

@node()
class Spi(NodeModel):
    device = CharField()
    rcc = CharField()
    interrupts = CharField()
    status = CharField()
    tx_dma = Cell(Dma.cell_class(), column_name="tx-dma")
    rx_dma = Cell(Dma.cell_class(), column_name="rx-dma")
    pinctrl = Cell(SpiPinctrl.cell_class(), null=True)

@node()
class I2CPinctrl(Pinctrl):
    sda = Cell(Gpio.cell_class())
    scl = Cell(Gpio.cell_class())

@node()
class I2C(NodeModel):
    device = CharField()
    rcc = CharField()
    interrupts = CharField()
    isr = CharField()
    status = CharField()
    tx_dma = Cell(Dma.cell_class(), column_name="tx-dma")
    rx_dma = Cell(Dma.cell_class(), column_name="rx-dma")
    speed = CharField()
    pinctrl = Cell(I2CPinctrl.cell_class(), null=True)
