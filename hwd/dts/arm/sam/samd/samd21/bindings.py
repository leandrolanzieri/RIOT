import logging
from hwd.dts.common import PhandleTo, String, Number, Pinctrl, node, Peripheral, PeripheralStatus
from hwd.dts.arm.v7m_nvic import Nvic

@node
class Sercom(Peripheral):
    """Sercom peripheral Device Tree binding for SAMD architecture.
    """
    device = String()

@node
class Gpio(Peripheral):
    """GPIO port Device Tree binding for SAMD architecture.
    """
    label = String()

    class CellData:
        cells = ['num', 'flags']

@node
class UsartPinctrl(Pinctrl):
    tx = PhandleTo(Gpio)
    rx = PhandleTo(Gpio)

@node
class Usart(Peripheral):
    sercom = PhandleTo(Sercom)
    clk = String()
    pinctrl = PhandleTo(UsartPinctrl)
    status = PeripheralStatus()