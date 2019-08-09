import fdt
from peewee import *
import peewee

db = SqliteDatabase(':memory:')

classes = {'usart', Usart}

class Usart(Model):
    device = CharField()
    bus = CharField()
    rcc = CharField()
    interrupts = CharField()
    isr = CharField()
    status = CharField()

    class Meta:
        database = db


dtb_data = open('board.dtb', 'rb').read()

tree = fdt.parse_dtb(dtb_data)
root = tree.root_node
phandles = tree.search('phandle')

phs = {}
for p in phandles:
    phs[p.data[0]] = p.path

print('== Phandles ==')
print(phs)
print('\n')

Usart.create_table()

for w in tree.walk():
    path = w[0]
    base = path.split('/')[-1]
    if base:
        dev = base.split('@')
        if dev[0] == 'usart':
            node = {}
            for p in tree.get_node(path).props:
                node[p.name] = p.data[0] if len(p.data) == 1 else p.data
            Usart.create(**node)

print("== All USARTs ==")
for u in Usart.select():
    print(u.device)
    print(u.interrupts)

print("\n== All active USARTs ==")
for u in Usart.select().where(Usart.status == 'okay'):
    print(u.device)
    print(u.status)
