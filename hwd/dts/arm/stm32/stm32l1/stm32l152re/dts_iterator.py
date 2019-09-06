import fdt
from peewee import *
import re
import ruamel.yaml as yaml
from jinja2 import Environment, FileSystemLoader, select_autoescape
import os

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
        model_class = _nodes.get(model_type, None)

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

class Pinmap(Model):
    id = AutoField()
    label = CharField()
    pin = CharField()
    group = CharField()

    @property
    def L(self):
        return self.label

    class Meta:
        database = db

class Pinout(Model):
    pin = CharField(primary_key=True)
    function = CharField()
    config_group = CharField()

    class Meta:
        database = db

    @classmethod
    def bulk_from_dict(cls, dictionary):
        cls._meta.database.create_tables([cls])
        for el in dictionary:
            cls.create(**el)

db.create_tables([Pinmap])
db.create_tables([Pinout])

with open('board.yml') as stream:
    board = yaml.safe_load(stream)
for k, v in board['board']['pinmap'].items():
    for el in v:
        el['group'] = k
        Pinmap.create(**el)

phs = {}
for p in phandles:
    Phandle.create(id=p.data[0], path=p.path)
    phs[p.data[0]] = p.path

for p in _nodes.values():
    p.create_table()

for w in tree.walk():
    path = w[0]
    Entry.get_model(path)

for p in tree.get_node("/chosen").props:
    config_group = p.name.split(",")[1].upper()
    path = Phandle.get(id=p.data[0]).path
    model = Entry.get_model(path)
    if model.status != 'okay':
        raise NotImplementedError
    for p in model.pins:
        el = {'pin': p['pin'], 'function': p['function'], 'config_group': config_group}
        Pinout.create(**el)

def format_function(function, config_group):
    return config_group + '.' + function

q = Pinmap.select(Pinmap.label.alias('L'), Pinmap.pin, Pinmap.group,
format_function(Pinout.function, Pinout.config_group) \
.alias('F')) \
.join(Pinout, JOIN.LEFT_OUTER, on=(Pinmap.pin == Pinout.pin)).dicts()

ctx = {}
for el in q:
    # remove None values so they don't get printed
    if el['F'] is None:
        del el['F']

    if not ctx.get(el['group']):
        # Connector numbering starts with 1 in boards, so we add padding
        ctx[el['group']] = [None, el]
    else:
        ctx[el['group']].append(el)

env = Environment(
    loader=FileSystemLoader(['.']),
    lstrip_blocks=True,
    trim_blocks=True
)

template_board = env.get_template('board.svg')
#board['board']['cpu_model'] = cpu['information']['model'].upper()
ctx['board'] = board

dirpath = os.getcwd()

file = open(dirpath + 'board.svg', 'w')
file.write(template_board.render(ctx))
