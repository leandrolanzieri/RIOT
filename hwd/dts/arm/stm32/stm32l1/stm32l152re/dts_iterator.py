import fdt
from peewee import *
import peewee
import re

db = SqliteDatabase(':memory:')

_periphs = {}
def periph(cls):
    _periphs[cls.__name__.lower()] = cls
    return cls

class PhandleCell(ForeignKeyField):
    def db_value(self, value):
        try:
            val = value[0]
        except:
            value = [value]
        path = Phandle.get(id=value[0]).url
        model = Entry.get_model(path)
        args = dict(zip(self.rel_model.CELLS, value[1:]))
        return super().db_value(self.rel_model.create(parent=model, **args).id)

@periph
class Dma(Model):
    id = CharField(primary_key=True)
    device = CharField()
    class Meta:
        database = db

class DmaCell(Model):
    CELLS = ['stream', 'channel']
    parent = ForeignKeyField(Dma)
    stream = IntegerField()
    channel = IntegerField()
    class Meta:
        database = db

@periph
class Gpio(Model):
    label = CharField()
    class Meta:
        database = db

class GpioCell(Model):
    CELLS = ['num']
    parent = ForeignKeyField(Gpio)
    num = IntegerField()

    class Meta:
        database = db

@periph
class UsartPinmux(Model):
    id = CharField(primary_key=True)
    tx = PhandleCell(GpioCell)
    tx_af = CharField()
    rx = PhandleCell(GpioCell)
    rx_af = CharField()

    class Meta:
        database = db

class UsartPinmuxCell(Model):
    CELLS = []
    parent = ForeignKeyField(UsartPinmux)
    class Meta:
        database = db

@periph
class Usart(Model):
    id = CharField(primary_key=True)
    device = CharField()
    rcc = CharField()
    interrupts = CharField()
    isr = CharField()
    status = CharField()
    tx_dma = PhandleCell(DmaCell)
    rx_dma = PhandleCell(DmaCell)
    pinmux = PhandleCell(UsartPinmuxCell, null=True)

    class Meta:
        database = db

class Phandle(Model):
    id = CharField(primary_key=True)
    url = CharField()
    class Meta:
        database = db

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
            node[p.name] = p.data[0] if len(p.data) == 1 else p.data

        return model_class.create(**node)

phandles = tree.search('phandle')

db.create_tables([Phandle])
db.create_tables([DmaCell])
db.create_tables([UsartPinmuxCell])
db.create_tables([GpioCell])

phs = {}
for p in phandles:
    Phandle.create(id=p.data[0], url=p.path)
    phs[p.data[0]] = p.path

for p in _periphs.values():
    p.create_table()

for w in tree.walk():
    path = w[0]
    Entry.get_model(path)

print("== All USARTs ==")
for u in Usart.select():
    print(u.device)
    print(u.interrupts)

print("\n== All active USARTs ==")
for u in Usart.select().where(Usart.status == 'okay'):
    print(u.device)
    print(u.status)
    print(u.pinmux.parent.tx_af)

for d in DmaCell.select():
    print("HOLA: ", d.stream)
