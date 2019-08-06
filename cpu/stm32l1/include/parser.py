import yaml
from jinja2 import Environment, FileSystemLoader
from peewee import *
import peewee
from playhouse.shortcuts import *

env = Environment(
    loader=FileSystemLoader('.'),
    lstrip_blocks=True,
    trim_blocks=True
)

template = env.get_template('stm32.jinja')
db = SqliteDatabase(':memory:')

with open('stm32l151x6.yml') as stream:
    try:
        af = yaml.safe_load(stream)

    except yaml.YAMLError as exc:
        print(exc)

with open('periph_conf.yml') as stream:
    try:
        pc = yaml.safe_load(stream)

    except yaml.YAMLError as exc:
        print(exc)

class BaseModel(Model):
    id = CharField(primary_key=True)
    class Meta:
        database = db

    @classmethod
    def create(cls, *args, **kwargs):
        o = cls(**kwargs)
        if "id" not in kwargs:
            o.id = o.get_label()
        o.save(force_insert=True)


class AlternateFunction(CharField):
    pass

class Periph(ForeignKeyField):
    pass

class CpuPin(BaseModel):
    port = CharField()
    num = IntegerField()

class Dma(BaseModel):
    type = CharField()
    device = CharField()

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

class Pin(ForeignKeyField):
    def __init__(self, **kwargs):
        super().__init__(CpuPin, **kwargs)

class UsartPinmux(BaseModel):
    periph = Periph(Usart, backref='relation')
    tx_pin = Pin()
    rx_pin = Pin()
    tx_af = AlternateFunction()
    rx_af = AlternateFunction()

    def get_pins(self):
        return {"tx_pin": self.tx_pin, "rx_pin": self.rx_pin}

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

class SpiPinmux(BaseModel):
    periph = ForeignKeyField(Spi)
    mosi_pin = Pin()
    miso_pin = Pin()
    af = CharField()

    def get_pins(self):
        return {"miso_pin": self.miso_pin, "mosi_pin": self.mosi_pin}

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

class I2CPinmux(BaseModel):
    periph = ForeignKeyField(I2C)
    scl_pin = Pin()
    sda_pin = Pin()
    scl_af = CharField()
    sda_af = CharField()

    def get_label(self):
        return "{}_scl_{}_{}_sda_{}_{}".format(
                self.periph,
                self.scl_pin.port, self.scl_pin.num,
                self.sda_pin.port, self.sda_pin.num)

    def get_pins(self):
        l = {"scl_pin": self.scl_pin}
        l.update(self.pinmux.get_pins())
        return l

class BasePinmuxConfig(Model):
    class Meta:
        database = db

    @classmethod
    def create(cls, **kwargs):
        pinmux_class = cls.pinmux.rel_model
        pinmux_data = kwargs.pop("pinmux")

        # Check if pinmux exists. Create if not
        pinmux_id = pinmux_data.pop("id", None)

        if (pinmux_id):
            pinmux = pinmux_class.select().where(pinmux_class.id==pinmux_id).get()
            if pinmux_data:
                pinmux.update(**pinmux_data).execute()
        else:
            pinmux = pinmux_class(**pinmux_data)
            pinmux.id = pinmux.get_label()
            pinmux.save(force_insert=True)

        sp = cls(pinmux=pinmux, **kwargs)
        sp.save(force_insert=True)
        return sp

    def get_pins(self):
        raise NotImplemented()

class I2CConfig(BasePinmuxConfig):
    pinmux = ForeignKeyField(I2CPinmux)
    speed = CharField()

class UsartConfig(BasePinmuxConfig):
    pinmux = ForeignKeyField(UsartPinmux)

    def get_pins(self):
        return self.pinmux.get_pins()

class SpiConfig(BasePinmuxConfig):
    id = AutoField()
    pinmux = ForeignKeyField(SpiPinmux)
    cs_pin = Pin()

    def get_pins(self):
        l = {"cs_pin": self.cs_pin}
        l.update(self.pinmux.get_pins())
        return l
    
class Pinout(Model):
    pin = Pin(primary_key=True)
    function = CharField()

    class Meta:
        db = database

db.create_tables([UsartPinmux, Usart])
db.create_tables([SpiPinmux, Spi])
db.create_tables([I2CPinmux, I2C])
db.create_tables([Dma])
db.create_tables([CpuPin])
db.create_tables([UsartConfig, SpiConfig, I2CConfig])

for el in af['pins']:
    CpuPin.create(**el)

for el in af["usart"]:
    Usart.create(**el)

for el in af["spi"]:
    Spi.create(**el)

for el in af["i2c"]:
    I2C.create(**el)

for el in af['i2c_pinmux']:
    I2CPinmux.create(**el)

for el in af['spi_pinmux']:
    SpiPinmux.create(**el)

for el in af['usart_pinmux']:
    UsartPinmux.create(**el)

for el in pc['board']['periph_conf']['i2c']:
    I2CConfig.create(**el)

for el in pc['board']['periph_conf']['spi']:
    SpiConfig.create(**el)

for el in pc['board']['periph_conf']['uart']:
    UsartConfig.create(**el)

used_pins = []
for conf in SpiConfig.select():
    used_pins.extend(conf.get_pins().values())

for conf in I2CConfig.select():
    used_pins.extend(conf.get_pins().values())


for c in UsartConfig.select():
    pass
    #used_pins.extend(get_pins(c))


