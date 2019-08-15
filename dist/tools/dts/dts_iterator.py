import fdt
from peewee import *
from hwd.dts.common import db
from hwd.dts.common import _nodes
from hwd.dts.common import DTBParser
from hwd.dts.common import DTBTree

import ruamel.yaml as yaml
from jinja2 import Environment, FileSystemLoader, select_autoescape

import os

import hwd
from hwd import dts

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

with open(os.environ['BOARD_YML']) as stream:
    board = yaml.safe_load(stream)

for k, v in board['board']['pinmap'].items():
    for el in v:
        el['group'] = k
        Pinmap.create(**el)

parser = DTBParser('stm32l152re')

for n, v in _nodes.items():
    db.create_tables([v.cell_class()])

parser.load(os.environ['DTB'])

for p in parser.tree.get_node("/chosen").props:
    config_group = p.name.split(",")[1].upper()
    path = DTBTree.get(key="phandle", value=p.data[0]).path
    model = parser.get_model(path)
    if model.status != 'okay':
        raise NotImplementedError

    if hasattr(model, 'pinctrl'):
        for k,v in model.pinctrl.target.cells.items():
            pin_label = "{}{}".format(v.target.label, v.num)
            el = {'pin': pin_label, 'function': k, 'config_group': config_group}
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

svg_file = os.environ['BOARD_SVG']
template_dir = os.path.dirname(svg_file)
template_base = os.path.basename(svg_file)

env = Environment(
    loader=FileSystemLoader([template_dir]),
    lstrip_blocks=True,
    trim_blocks=True
)

template_board = env.get_template(template_base)
#board['board']['cpu_model'] = cpu['information']['model'].upper()
ctx['board'] = board
print(template_board.render(ctx))
