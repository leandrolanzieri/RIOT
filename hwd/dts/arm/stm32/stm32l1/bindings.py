from hwd.dts.common import  PhandleTo, String, Pinctrl, node, Peripheral

@node
class Dma(Peripheral):
    device = String()

    class CellData:
        cells = ['channel', 'request']

@node
class Rcc(Peripheral):
    class CellData:
        cells = ['bus', 'bits']

@node
class Gpio(Peripheral):
    label = String()

    class CellData:
        cells = ['num', 'flags']

@node
class UsartPinctrl(Pinctrl):
    tx = PhandleTo(Gpio)
    rx = PhandleTo(Gpio)

@node
class Usart(Peripheral):
    device = String()
    rcc = PhandleTo(Rcc)
    interrupts = String()
    isr = String()
    status = String()
    tx_dma = PhandleTo(Dma, column_name="tx-dma")
    rx_dma = PhandleTo(Dma, column_name="rx-dma")
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

        if self.pinctrl:
            tx = self.pinctrl.target.tx
            rx = self.pinctrl.target.rx
            ret['tx_pin'] = 'GPIO_PIN({},{})'.format(tx.target.label, tx.num)
            ret['rx_pin'] = 'GPIO_PIN({},{})'.format(tx.target.label, rx.num)
            ret['tx_af'] = tx.flags
            ret['rx_af'] = rx.flags

        return ret

@node
class SpiPinctrl(Pinctrl):
    miso = PhandleTo(Gpio)
    mosi = PhandleTo(Gpio)
    sck = PhandleTo(Gpio)
    cs = PhandleTo(Gpio, null=True)

@node
class Spi(Peripheral):
    device = String()
    rcc = PhandleTo(Rcc)
    interrupts = String()
    status = String()
    tx_dma = PhandleTo(Dma, column_name="tx-dma")
    rx_dma = PhandleTo(Dma, column_name="rx-dma")
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

        if self.pinctrl:
            mosi = self.pinctrl.target.mosi
            miso = self.pinctrl.target.miso
            sck = self.pinctrl.target.sck
            ret['mosi_pin'] = 'GPIO_PIN({},{})'.format(mosi.target.label, mosi.num)
            ret['miso_pin'] = 'GPIO_PIN({},{})'.format(miso.target.label, miso.num)
            ret['sck_pin'] = 'GPIO_PIN({},{})'.format(sck.target.label, sck.num)
            ret['af'] = mosi.flags
            if self.pinctrl.target.cs:
                cs = self.pinctrl.target.cs
                ret['cs_pin'] = 'GPIO_PIN({},{})'.format(cs.target.label, cs.num)
            else:
                ret['cs_pin'] = 'GPIO_UNDEF'

        return ret

@node
class I2CPinctrl(Pinctrl):
    sda = PhandleTo(Gpio)
    scl = PhandleTo(Gpio)

@node
class I2C(Peripheral):
    device = String()
    rcc = PhandleTo(Rcc)
    interrupts = String()
    isr = String()
    status = String()
    tx_dma = PhandleTo(Dma, column_name="tx-dma")
    rx_dma = PhandleTo(Dma, column_name="rx-dma")
    speed = String(null=True)
    clk = String(null=True)
    pinctrl = PhandleTo(I2CPinctrl, null=True)

    def render(self):
        ret = {}
        ret['dev'] = self.device
        ret['speed'] = self.speed
        ret['rcc_mask'] = self.rcc.bits
        ret['bus'] = self.rcc.bus
        ret['irqn'] = self.interrupts
        ret['clk'] = self.clk
        ret['tx_dma_request'] = self.tx_dma.request
        ret['tx_dma_channel'] = self.tx_dma.channel
        ret['rx_dma_request'] = self.rx_dma.request
        ret['rx_dma_channel'] = self.rx_dma.channel

        if self.pinctrl:
            scl = self.pinctrl.target.scl
            sda = self.pinctrl.target.sda
            ret['scl_pin'] = 'GPIO_PIN({},{})'.format(scl.target.label, scl.num)
            ret['scl_af'] = scl.flags
            ret['sda_pin'] = 'GPIO_PIN({},{})'.format(sda.target.label, sda.num)
            ret['sda_af'] = sda.flags

        return ret
