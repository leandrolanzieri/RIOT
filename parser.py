import os
import json
from jinja2 import Environment, FileSystemLoader, select_autoescape
import yaml
from peewee import *
import peewee
from playhouse.shortcuts import *
from dts.stm32.stm32l1 import *


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
        return self.pinmux.periph.type + str(self.id - 1)

    def get_pins(self):
        pins = self.pinmux.get_pins()
        for p in pins:
            p['config_group'] = self.config_group
        return pins

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

    for el in cpu["peripherals"]["usart"]:
        Usart.create(**el)

    for el in cpu["peripherals"]["spi"]:
        Spi.create(**el)

    for el in cpu["peripherals"]["i2c"]:
        I2C.create(**el)

    for el in cpu["peripherals"]['pinmuxes']['i2c_pinmux']:
        I2CPinmux.create(**el)

    for el in cpu["peripherals"]['pinmuxes']['spi_pinmux']:
        SpiPinmux.create(**el)

    for el in cpu["peripherals"]['pinmuxes']['usart_pinmux']:
        UsartPinmux.create(**el)

    for el in board['board']['periph_conf']['i2c']:
        I2CConfig.create(**el)

    for el in board['board']['periph_conf']['spi']:
        SpiConfig.create(**el)

    for el in board['board']['periph_conf']['uart']:
        UsartConfig.create(**el)
    
    for k, v in board['board']['pinmap'].items():
        for el in v:
            el['connector'] = k
            Pinmap.create(**el)

if __name__ == "__main__":

    env = Environment(
        loader=FileSystemLoader([os.environ['RIOTBASE'] + 'hwd/dts/', '.']),
        lstrip_blocks=True,
        trim_blocks=True
    )

    with open(os.environ['RIOTBOARD'] + 'board.yml') as stream:
        board = yaml.safe_load(stream)

    template_cpu = env.get_template(board['board']['cpu'])
    cpu = yaml.safe_load(template_cpu.render())['cpu']
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

    q = Pinmap.select(Pinmap.label.alias('L'), Pinmap.pin, Pinmap.connector,
                      format_function(Pinout.function, Pinout.config_group) \
                      .alias('F')) \
              .join(Pinout, JOIN.LEFT_OUTER, on=(Pinmap.pin == Pinout.pin)).dicts()

    for el in q:
        # remove None values so they don't get printed
        if el['F'] is None:
            del el['F']

        if not board.get(el['connector']):
            # Connector numbering starts with 1 in boards, so we add padding
            board[el['connector']] = [None, el]
        else:
            board[el['connector']].append(el)

    template_board = env.get_template('board.svg')
    board['board']['cpu_model'] = cpu['information']['model'].upper()
    print(template_board.render(board))
