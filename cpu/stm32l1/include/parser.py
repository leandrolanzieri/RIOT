import json
from jinja2 import Environment, FileSystemLoader, select_autoescape
import yaml
from peewee import *
import peewee
from playhouse.shortcuts import *

db = SqliteDatabase(':memory:')

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

    def get_label(self):
        return "P{}{}".format(self.port, self.num)

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

class BasePinmux(BaseModel):
    def get_pins(self):
        l = []
        for k,v in self._meta.fields.items():
            if isinstance(v, Pin):
                l.append({'pin': getattr(self, k), 'function': k})
        return l

class UsartPinmux(BasePinmux):
    periph = Periph(Usart, backref='relation')
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

class BasePinmuxConfig(Model):
    id = AutoField()
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

    @property
    def config_group(self):
        return self.pinmux.periph.type + str(self.id)

    def get_pins(self):
        pins = self.pinmux.get_pins()
        for p in pins:
            p['config_group'] = self.config_group
        return pins

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

class Pinout(Model):
    pin = Pin(primary_key=True)
    function = CharField()
    config_group = CharField()

    class Meta:
        database = db

class Pinmap(Model):
    id = AutoField()
    label = CharField()
    pin = CharField()
    connector = CharField()

    @property
    def L(self):
        return self.label

    class Meta:
        database = db



def create_tables():
    db.create_tables([UsartPinmux, Usart])
    db.create_tables([SpiPinmux, Spi])
    db.create_tables([I2CPinmux, I2C])
    db.create_tables([Dma])
    db.create_tables([CpuPin])
    db.create_tables([UsartConfig, SpiConfig, I2CConfig])
    db.create_tables([Pinout])
    db.create_tables([Pinmap])

def format_function(function, config_group):
    return config_group + '.' + function

def create_elements(cpu, board):
    for el in cpu['pins']:
        CpuPin.create(**el)

    for el in cpu["usart"]:
        Usart.create(**el)

    for el in cpu["spi"]:
        Spi.create(**el)

    for el in cpu["i2c"]:
        I2C.create(**el)

    for el in cpu['i2c_pinmux']:
        I2CPinmux.create(**el)

    for el in cpu['spi_pinmux']:
        SpiPinmux.create(**el)

    for el in cpu['usart_pinmux']:
        UsartPinmux.create(**el)

    for el in board['board']['periph_conf']['i2c']:
        I2CConfig.create(**el)

    for el in board['board']['periph_conf']['spi']:
        SpiConfig.create(**el)

    for el in board['board']['periph_conf']['uart']:
        UsartConfig.create(**el)
    
    for k, v in board['board']['pin_map'].items():
        for el in v:
            el['connector'] = k
            Pinmap.create(**el)

if __name__ == "__main__":

    with open('stm32l151x6.yml') as stream:
        try:
            cpu = yaml.safe_load(stream)

        except yaml.YAMLError as exc:
            print(exc)

    with open('periph_conf.yml') as stream:
        try:
            board = yaml.safe_load(stream)

        except yaml.YAMLError as exc:
            print(exc)

    create_tables()
    create_elements(cpu, board)

    used_pins = []
    for conf in SpiConfig.select():
        used_pins.extend(conf.get_pins())

    for conf in I2CConfig.select():
        used_pins.extend(conf.get_pins())

    for conf in UsartConfig.select():
        used_pins.extend(conf.get_pins())

    for pin in used_pins:
        Pinout.create(**pin)

    q = Pinmap.select(Pinmap.label, Pinmap.pin, Pinmap.connector,
                      format_function(Pinout.function, Pinout.config_group) \
                      .alias('function')) \
              .join(Pinout, JOIN.LEFT_OUTER, on=(Pinmap.pin == Pinout.pin)).dicts()

    for el in q:
        if not board.get(el['connector']):
            # Connector numbering starts with 1 in boards, so we add padding
            board[el['connector']] = [None, el]
        else:
            board[el['connector']].append(el)

    env = Environment(
        loader=FileSystemLoader('.'),
        lstrip_blocks=True,
        trim_blocks=True,
        autoescape=select_autoescape(['xml', 'html'])
    )

    template_board = env.get_template('Nucleo-64_nv_plain.svg')

    print(template_board.render(board))