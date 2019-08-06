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

        o.save()

class Pin(BaseModel):
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

class Pinout(ForeignKeyField):
    def __init__(self, **kwargs):
        super().__init__(Pin, **kwargs)

class UsartPinmux(BaseModel):
    periph = ForeignKeyField(Usart, backref='relation')
    tx_pin = Pinout()
    rx_pin = Pinout()
    tx_af = CharField()
    rx_af = CharField()

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
    mosi_pin = Pinout()
    miso_pin = Pinout()
    af = CharField()

    def get_pins(self):
        return {"miso_pin": self.miso_pin, "mosi_pin": self.mosi_pin}

    def get_label(self):
        return "{}_mosi_{}_{}_miso_{}_{}".format(
                self.periph,
                self.mosi_pin.port, self.mosi_pin.num,
                self.miso_pin.port, self.miso_pin.num)

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
    scl_pin = Pinout()
    sda_pin = Pinout()
    scl_af = CharField()
    sda_af = CharField()

    def get_label(self):
        return "{}_scl_{}_{}_sda_{}_{}".format(
                self.periph,
                self.scl_pin.port, self.scl_pin.num,
                self.sda_pin.port, self.sda_pin.num)

class PeriphConfig(BaseModel):
    uart = ForeignKeyField(UsartPinmux)
    spi = ForeignKeyField(SpiPinmux)
    i2c = ForeignKeyField(I2CPinmux)

class I2CConfig(Model):
    pinmux = ForeignKeyField(I2CPinmux)
    speed = CharField()
    class Meta:
        database = db

    @classmethod
    def create_conf(*args, **kwargs):
        pinmux_data = kwargs["pinmux"]
        if(pinmux_data.get("base")):
            pinmux = I2CPinmux.select().where(I2CPinmux.id==pinmux_data["base"]).get()
            pinmux_data.pop("base")
            pinmux.update(**pinmux_data).execute()
        else:
            pinmux = I2CPinmux(**pinmux_data)
            pinmux.id = pinmux.get_label()
            pinmux.save()

        kwargs.pop("pinmux")

        I2CConfig.create(pinmux=pinmux, **kwargs)

class UsartConfig(Model):
    pinmux = ForeignKeyField(UsartPinmux)
    class Meta:
        database = db

    def get_pins(self):
        return self.pinmux.get_pins()

    @classmethod
    def create_conf(*args, **kwargs):
        if(kwargs.get("base")):
            pinmux = UsartPinmux.select().where(UsartPinmux.id==kwargs["base"]).get()
            kwargs.pop("base")
            pinmux.update(**kwargs).execute()
        else:
            pinmux = UsartPinmux(**kwargs)
            pinmux.id = pinmux.get_label()
            pinmux.save()

        SpiConfig.create(cs_pin=cs_pin, pinmux=pinmux)

class BasePinmuxConfig(Model):
    @classmethod
    def create(cls, **kwargs):
        base = kwargs.pop("pinmux_base", None)
        pinmux_class = cls.pinmux.rel_model
        pinmux_data = kwargs.pop("pinmux", None)

        if(base):
            pinmux = pinmux_class.select().where(pinmux_class.id==base).get()
            if(pinmux_data):
                pinmux.update(**pinmux_data).execute()
        else:
            pinmux = pinmux_class(**pinmux_data)
            pinmux.id = pinmux.get_label()
            pinmux.save()

        sp = cls(pinmux=pinmux, **kwargs)
        sp.save()
        print(sp.pinmux)


class SpiConfig(BasePinmuxConfig):
    id = AutoField()
    pinmux = ForeignKeyField(SpiPinmux)
    cs_pin = Pinout()
    class Meta:
        database = db

    def get_pins(self):
        l = {"cs_pin": self.cs_pin}
        l.update(self.pinmux.get_pins())
        return l
    
        
db.create_tables([UsartPinmux, Usart])
db.create_tables([SpiPinmux, Spi])
db.create_tables([I2CPinmux, I2C])
db.create_tables([Dma])
db.create_tables([Pin])
db.create_tables([UsartConfig, SpiConfig, I2CConfig])

for el in af['pins']:
    Pin.create(**el)

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

#for el in pc['board']['periph_conf']['i2c']:
#    I2CConfig.create(**el)

for el in pc['board']['periph_conf']['spi']:
    SpiConfig.create(**el)
    #sp = SpiConfig.create(**el)

#for el in pc['board']['periph_conf']['uart']:
#    UsartConfig.create(**el)

used_pins = []
def get_pins(model):
    l = []
    for k, v in model.__class__.__dict__.items():
        if isinstance(v, peewee.ForeignKeyAccessor) and v.rel_model == Pin:
            l.append(getattr(model, k))
    return l

for conf in SpiConfig.select():
    print(conf.pinmux.miso_pin)
    print(conf.pinmux.mosi_pin)

for c in I2CConfig.select():
    used_pins.extend(get_pins(c))

for c in UsartConfig.select():
    used_pins.extend(get_pins(c))


