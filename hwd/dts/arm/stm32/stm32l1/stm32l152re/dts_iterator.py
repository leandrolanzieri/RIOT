import fdt
from peewee import *
import peewee
import re

db = SqliteDatabase(':memory:')

_periphs = {}
def node(cells=[]):
    def inner(cls):
        d = {"CELLS": cells, "parent":ForeignKeyField(cls)}
        for c in cells:
            d[c] =  IntegerField()

        __cls = type("{}Cell".format(cls.__name__),
                (BaseModel,), d)

        db.create_tables([__cls])
        setattr(cls, __cls.__name__, __cls)
        _periphs[cls.__name__.lower()] = cls
        return cls
    return inner

class BaseModel(Model):
    class Meta:
        database = db

class Cell(ForeignKeyField):
    def __init__(self, model, **kwargs):
        super().__init__(getattr(model, "{}Cell".format(model.__name__)), **kwargs)

    def db_value(self, value):
        try:
            val = value[0]
        except:
            value = [value]
        path = Phandle.get(id=value[0]).url
        model = Entry.get_model(path)
        args = dict(zip(self.rel_model.CELLS, value[1:]))
        return super().db_value(self.rel_model.create(parent=model, **args).id)

class Phandle(Model):
    id = CharField(primary_key=True)
    url = CharField()
    class Meta:
        database = db

###############################################################################

@node(cells=['stream', 'channel'])
class Dma(Model):
    id = CharField(primary_key=True)
    device = CharField()
    class Meta:
        database = db

@node(cells=['num', 'flags'])
class Gpio(BaseModel):
    label = CharField()

@node()
class UsartPinctrl(BaseModel):
    id = CharField(primary_key=True)
    tx = Cell(Gpio)
    rx = Cell(Gpio)

@node()
class Usart(BaseModel):
    id = CharField(primary_key=True)
    device = CharField()
    rcc = CharField()
    interrupts = CharField()
    isr = CharField()
    status = CharField()
    tx_dma = Cell(Dma, column_name="tx-dma")
    rx_dma = Cell(Dma, column_name="rx-dma")
    pinctrl = Cell(UsartPinctrl, null=True)

@node()
class SpiPinctrl(BaseModel):
    id = CharField(primary_key=True)
    miso = Cell(Gpio)
    mosi = Cell(Gpio)
    sck = Cell(Gpio)
    cs = Cell(Gpio, null=True)

@node()
class Spi(BaseModel):
    id = CharField(primary_key=True)
    device = CharField()
    rcc = CharField()
    interrupts = CharField()
    status = CharField()
    tx_dma = Cell(Dma, column_name="tx-dma")
    rx_dma = Cell(Dma, column_name="rx-dma")
    pinctrl = Cell(SpiPinctrl, null=True)

@node()
class I2CPinctrl(BaseModel):
    id = CharField(primary_key=True)
    sda = Cell(Gpio)
    scl = Cell(Gpio)

@node()
class I2C(BaseModel):
    id = CharField(primary_key=True)
    device = CharField()
    rcc = CharField()
    interrupts = CharField()
    isr = CharField()
    status = CharField()
    tx_dma = Cell(Dma, column_name="tx-dma")
    rx_dma = Cell(Dma, column_name="rx-dma")
    pinctrl = Cell(I2CPinctrl, null=True)
    speed = CharField()


dtb_data = open('board.dtb', 'rb').read()
tree = fdt.parse_dtb(dtb_data)
root = tree.root_node

class Entry:
    @staticmethod
    def get_model(path):
        m = re.search("\/([\w]*)@(.*)$", path)
        if not m:
            return None

        model_type = m[1]
        model_id = m[2]
        model_class = _periphs.get(model_type, None)

        if not model_class:
            return None

        # Check if model exists. Create if not
        if(len(model_class.select().where(model_class.id == model_id)) > 0):
            return model_class.get(model_class.id == model_id)

        node = {"id": model_id}

        for p in tree.get_node(path).props:
            if hasattr(p, 'data'):
                node[p.name] = p.data[0] if len(p.data) == 1 else p.data
            else:
                node[p.name] = True

        return model_class.create(**node)

phandles = tree.search('phandle')

db.create_tables([Phandle])

phs = {}
for p in phandles:
    Phandle.create(id=p.data[0], url=p.path)
    phs[p.data[0]] = p.path

for p in _periphs.values():
    p.create_table()

for w in tree.walk():
    path = w[0]
    Entry.get_model(path)

print(db.get_tables())
print("== All USARTs ==")
for u in Usart.select():
    print(u.device)
    print(u.interrupts)

print("\n== All active USARTs ==")
for u in Usart.select().where(Usart.status == 'okay'):
    print(u.device)
    print(u.status)
    print(u.pinctrl.parent.tx)

for u in Spi.select().where(Spi.status == 'okay'):
    print(u.device)
    print(u.status)

for u in I2C.select().where(I2C.status == 'okay'):
    print(u.device)
    print(u.status)

