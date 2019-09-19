import logging
from hwd.dts.common import  PhandleTo, String, Number, Pinctrl, node, Peripheral, PeripheralStatus
from hwd.dts.arm.v7m_nvic import Nvic

@node
class Dma(Peripheral):
    """DMA peripheral Device Tree binding for STM32 architecture.

    Properties:
        - device: String representing the device (e.g. DMA1)
    """
    device = String()

    class CellData:
        """Phandles that reference DMAs.

        Cells:
            - channel: DMA Channel number
            - request: DMA Request number
        """
        cells = ['channel', 'request']

@node
class Rcc(Peripheral):
    """Reset and Clock control peripheral Device Tree bindinf for STM32
    architecture.
    """

    class CellData:
        """Phandles that reference the RCC.

        Cells:
            - bus: Clock bus (e.g. APB1)
            - bits: Bits to set in the RCC (e.g. RCC_APB1ENR_SPI3EN).
        """
        cells = ['bus', 'bits']

@node
class Gpio(Peripheral):
    """GPIO port controller Device Tree binding for STM32 architecture.

    Properties:
        - label: String representing the port (e.g. PORT_A)
    """
    label = String()

    class CellData:
        """Phandles that reference a GPIO port

        Cells:
            - num: Pin number in the port
            - flags: Flags needed for pin configuration (e.g. alternate
                     functions needed)
        """
        cells = ['num', 'flags']

@node
class AdcPinctrl(Pinctrl):
    """Pinctrl for ADC peripheral Device Tree bindings, for STM32 architecture.

    Pins:
        - pin: ADC input pin
    """
    pin = PhandleTo(Gpio)

    def render(self):
        ret = {}
        ret['pin'] = self.render_pin(self.pin)
        return ret

@node
class AdcChannel(Peripheral):
    """ADC channel Device Tree binding for STM32 architecture.

    Properties:
        - status: Peripheral activation status
        - channel: ADC channel number
        - pinctrl: Pin configuration for the ADC
    """
    _node_name = 'adc-ch'
    status = PeripheralStatus()
    channel = Number()
    pinctrl = PhandleTo(AdcPinctrl)

    def render(self):
        ret = {}
        ret['chan'] = self.channel
        ret.update(self.pinctrl.target.render())
        return ret

@node
class Adc(Peripheral):
    """ADC peripheral Device Tree binding for STM32 architecture

    Properties:
        - device: String representing the device (e.g. ADC1)
        - dma: ADC DMA configuration
    """
    device = String()
    dma = PhandleTo(Dma)

@node
class UsartPinctrl(Pinctrl):
    """Pinctrl for USART peripherals Device Tree bindings, for STM32
    architecture.

    Pins:
        - tx: USART Transmission
        - rx: USART Reception
    """
    tx = PhandleTo(Gpio)
    rx = PhandleTo(Gpio)

    def render(self):
        ret = {}
        ret['tx_pin'] = self.render_pin(self.tx)
        ret['rx_pin'] = self.render_pin(self.rx)
        ret['tx_af'] = self.tx.flags
        ret['rx_af'] = self.rx.flags
        return ret

@node
class Usart(Peripheral):
    """USART peripheral Device Tree binding for STM32 architecture.

    Properties:
        - device: String representing the device (e.g. USART1)
        - rcc: Phandle to the RCC
        - interrupts: Phandle to the NVIC
        - isr: String of the interrupt vector
        - status: Peripheral activation status
        - tx_dma: USART Transmission DMA configuration
        - rx_dma: USART Reception DMA configuration
        - pinctrl: Pin configuration for the USART
    """
    device = String()
    rcc = PhandleTo(Rcc)
    interrupts = PhandleTo(Nvic)
    isr = String()
    status = PeripheralStatus()
    tx_dma = PhandleTo(Dma)
    rx_dma = PhandleTo(Dma)
    pinctrl = PhandleTo(UsartPinctrl, null=True)

    def render(self):
        ret = {}
        ret['dev'] = self.device
        ret['bus'] = self.rcc.bus
        ret['rcc_mask'] = self.rcc.bits
        ret['tx_dma_request'] = self.tx_dma.request
        ret['tx_dma_channel'] = self.tx_dma.channel
        ret['rx_dma_request'] = self.rx_dma.request
        ret['rx_dma_channel'] = self.rx_dma.channel

        try:
            ret.update(self.pinctrl.target.render())
        except AttributeError:
            logging.warning('{} has no assigned pins'.format(self.device))
        return ret

@node
class SpiPinctrl(Pinctrl):
    """Pinctrl for SPI peripherals Device Tree bindings, for STM32
    architecture.

    Pins:
        - miso: SPI Master Input Slave Output
        - mosi: SPI Master Output Slave Input
        - sck: SPI Clock
        - cs: SPI Chip Select
    """
    miso = PhandleTo(Gpio)
    mosi = PhandleTo(Gpio)
    sck = PhandleTo(Gpio)
    cs = PhandleTo(Gpio, null=True)

    def render(self):
        ret = {}
        ret['mosi_pin'] = self.render_pin(self.mosi)
        ret['miso_pin'] = self.render_pin(self.miso)
        ret['sck_pin'] = self.render_pin(self.sck)
        ret['cs_pin'] = self.render_pin(self.cs)
        ret['af'] = self.mosi.flags
        return ret
@node
class Spi(Peripheral):
    """SPI peripheral Device Tree binding for STM32 architecture.

    Properties:
        - device: String representing the device (e.g. SPI1)
        - rcc: Phandle to the RCC
        - interrupts: Phandle to the NVIC
        - status: Peripheral activation status
        - tx_dma: SPI Transmission DMA configuration
        - rx_dma: SPI Reception DMA configuration
        - pinctrl: Pin configuration for the SPI
    """
    device = String()
    rcc = PhandleTo(Rcc)
    interrupts = PhandleTo(Nvic)
    status = PeripheralStatus()
    tx_dma = PhandleTo(Dma)
    rx_dma = PhandleTo(Dma)
    pinctrl = PhandleTo(SpiPinctrl, null=True)

    def render(self):
        ret = {}
        ret['dev'] = self.device
        ret['rccmask'] = self.rcc.bits
        ret['apbbus'] = self.rcc.bus
        ret['tx_dma_request'] = self.tx_dma.request
        ret['tx_dma_channel'] = self.tx_dma.channel
        ret['rx_dma_request'] = self.rx_dma.request
        ret['rx_dma_channel'] = self.rx_dma.channel

        try:
            ret.update(self.pinctrl.target.render())
        except AttributeError:
            logging.warning('{} has no assigned pins'.format(self.device))
        return ret

@node
class I2CPinctrl(Pinctrl):
    """Pinctrl for I2C peripherals Device Tree bindings, for STM32
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
        ret['scl_af'] = self.scl.flags
        ret['sda_af'] = self.sda.flags
        return ret

@node
class I2C(Peripheral):
    """I2C peripheral Device Tree binding for STM32 architecture.

    Properties:
        - device: String representing the device (e.g. I2C0)
        - rcc: Phandle to the RCC
        - interrupts: Phandle to the NVIC
        - isr: String of the interrupt vector
        - status: Peripheral activation status
        - tx_dma: I2C Transmission DMA configuration
        - rx_dma: I2C Reception DMA configuration
        - clk: Clock configuration
        - speed: Speed of the bus
        - pinctrl: Pin configuration for the I2C
    """
    device = String()
    rcc = PhandleTo(Rcc)
    interrupts = PhandleTo(Nvic)
    isr = String()
    status = PeripheralStatus()
    tx_dma = PhandleTo(Dma)
    rx_dma = PhandleTo(Dma)
    speed = String(null=True)
    clk = String(null=True)
    pinctrl = PhandleTo(I2CPinctrl, null=True)

    def render(self):
        ret = {}
        ret['dev'] = self.device
        ret['speed'] = self.speed
        ret['rcc_mask'] = self.rcc.bits
        ret['bus'] = self.rcc.bus
        ret['irqn'] = self.interrupts.line
        ret['clk'] = self.clk
        ret['tx_dma_request'] = self.tx_dma.request
        ret['tx_dma_channel'] = self.tx_dma.channel
        ret['rx_dma_request'] = self.rx_dma.request
        ret['rx_dma_channel'] = self.rx_dma.channel

        try:
            ret.update(self.pinctrl.target.render())
        except AttributeError:
            logging.warning('{} has no assigned pins'.format(self.device))
        return ret
