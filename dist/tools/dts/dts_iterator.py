import fdt
from peewee import *
from hwd.dts.common import db
from hwd.dts.common import _nodes
from hwd.dts.common import DTBParser
from hwd.dts.common import NodeModel
from hwd.dts.common import Pinmap
from hwd.dts.common import Pinout

import ruamel.yaml as yaml
from jinja2 import Environment, FileSystemLoader, select_autoescape

import os

import hwd
from hwd import dts

db.create_tables([Pinmap])
db.create_tables([Pinout])

with open(os.environ['BOARD_YML']) as stream:
    board = yaml.safe_load(stream)

for k, v in board['board']['pinmap'].items():
    for el in v:
        el['group'] = k
        Pinmap.create(**el)

parser = DTBParser('stm32l152re')
parser.load(os.environ['DTB'])
parser.create_pinout()

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
#print(template_board.render(ctx))
