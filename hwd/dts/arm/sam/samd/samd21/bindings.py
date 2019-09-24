import logging
from hwd.dts.common import PhandleTo, String, Number, Pinctrl, node, Peripheral, PeripheralStatus
from hwd.dts.arm.v7m_nvic import Nvic

@node
class Sercom(Peripheral):
    """Sercom peripheral Device Tree binding for SAMD architecture.
    """
    device = String()

    class CellData(Peripheral.CellData):
        unique = True

@node
class Gpio(Peripheral):
    """GPIO port Device Tree binding for SAMD architecture.
    """
    label = String()

    class CellData(Peripheral.CellData):
        cells = ['num', 'flags']

@node
class UsartPinctrl(Pinctrl):
    """Pinctrl for USART peripherals Device Tree bindings for SAMD architecture.

    Pins:
        - tx: USART Transmission
        - rx: USART Reception
    """
    tx = PhandleTo(Gpio)
    rx = PhandleTo(Gpio)

    def render(self):
        ret = {}
        ret['tx_pin'] = self.render_pin(self.tx)
        ret['tx_pad'] = self.tx.flags
        ret['rx_pin'] = self.render_pin(self.rx)
        ret['rx_pad'] = self.rx.flags
        return ret

@node
class Usart(Peripheral):
    """USART peripheral Device Tree bindings for SAMD architecture.

    Properties:
        - sercom: Serial Communication device used
        - clk: Clock control
        - pinctrl: Pin configuration for the USART
        - status: Peripheral activation status
        - flags: Extra configuration flags
    """
    sercom = PhandleTo(Sercom)
    clk = String()
    pinctrl = PhandleTo(UsartPinctrl)
    status = PeripheralStatus()
    flags = Number()

    def render(self):
        ret = {}
        # TODO: Add gpio mux
        # should this go here?
        ret['dev'] = '&{}->USART'.format(self.sercom.target.device)
        ret['gclk_src'] = self.clk
        ret['flags'] = self.flags
        ret.update(self.pinctrl.target.render())
        return ret

@node
class SpiPinctrl(Pinctrl):
    """Pinctrl for SPI peripherals Device Tree bindings for SAMD architecture.

    Pins:
        - miso: SPI Master Input Slave Output
        - mosi: SPI Master Output Slave Input
        - sck: SPI Clock
    """
    miso = PhandleTo(Gpio)
    mosi = PhandleTo(Gpio)
    sck = PhandleTo(Gpio)

    def render(self):
        ret = {}
        ret['miso_pin'] = self.render_pin(self.miso)
        ret['mosi_pin'] = self.render_pin(self.mosi)
        ret['clk_pin'] = self.render_pin(self.sck)
        ret['miso_pad'] = self.miso.flags
        ret['mosi_pad'] = self.mosi.flags
        return ret

@node
class Spi(Peripheral):
    """SPI peripheral Device Tree bindings for SAMD architecture.
    """
    sercom = PhandleTo(Sercom)
    pinctrl = PhandleTo(SpiPinctrl)
    status = PeripheralStatus()

    def render(self):
        ret = {}
        ret['dev'] = '&{}->SPI'.format(self.sercom.target.device)
        ret.update(self.pinctrl.target.render())
        return ret


@node
class I2CPinctrl(Pinctrl):
    """Pinctrl for I2C peripherals Device Tree bindings, for SAMD
    architecture.

    Pins:
        - sda: I2C Serial Data
        - scl: I2C Serial Clock
    """
    sda = PhandleTo(Gpio)
    scl = PhandleTo(Gpio)

    def render(self):
        ret = {}
        ret['scl_pin'] = self.render_pin(self.scl)
        ret['sda_pin'] = self.render_pin(self.sda)
        ret['mux'] = self.scl.flags
        return ret

@node
class I2C(Peripheral):
    """I2C peripheral Device Tree binding for SAMD architecture.

    Properties:
        - sercom: Reference to the SERCOM device used
        - status: Peripheral activation status
        - clk: Clock configuration
        - speed: Speed of the bus
        - pinctrl: Pin configuration for the I2C
    """
    sercom = PhandleTo(Sercom)
    status = PeripheralStatus()
    speed = String()
    clk = String()
    pinctrl = PhandleTo(I2CPinctrl)

    def render(self):
        ret = {}
        ret['dev'] = '&{}->I2CM'.format(self.sercom.target.device)
        ret['speed'] = self.speed
        ret['clk'] = self.clk
        ret.update(self.pinctrl.target.render())
        return ret
