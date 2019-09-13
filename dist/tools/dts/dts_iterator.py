import logging
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
import argparse

import hwd
from hwd import dts

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
            description='Perform operations on Device Trees and Bindings')

    parser.add_argument('dtb', help='Path to the .dtb file')
    parser.add_argument('-b', '--board', help='Board description file')
    parser.add_argument('--log', help='Logging level. Default: Warning.')
    args = parser.parse_args()

    # verify that the provided log level is valid
    if args.log:
        numeric_level = getattr(logging, args.log.upper(), None)
        if not isinstance(numeric_level, int):
            raise ValueError('Invalid log level: %s' % args.log)
        logging.basicConfig(level=numeric_level)

    db.create_tables([Pinmap])
    db.create_tables([Pinout])

    try:
        with open(args.board) as stream:
            board = yaml.safe_load(stream)

        for k, v in board['board']['pinmap'].items():
            for el in v:
                el['group'] = k
                Pinmap.create(**el)
    except TypeError:
        logging.info('No board descripcion provided')

    dtparser = DTBParser()
    dtparser.load(args.dtb)
    dtparser.create_pinout()

    def format_function(function, config_group):
        return config_group + '.' + function

    q = Pinmap.select(Pinmap.label.alias('L'), Pinmap.pin, Pinmap.group,
    format_function(Pinout.function, Pinout.config_group) \
    .alias('F')) \
    .join(Pinout, JOIN.LEFT_OUTER, on=(Pinmap.pin == Pinout.pin)).dicts()

    # This will get all I2C configurations and will render them using a generic
    # dictionary to C struct jinja template
    env2 = Environment(
        loader=FileSystemLoader([os.environ['DTS_TEMPLATES']]),
        lstrip_blocks=True,
        trim_blocks=True
    )
    template_struct = env2.get_template('array_of_structs.jinja')

    i2cs = dtparser.render_peripherals('i2c')
    ctx = {'attributes': ['static', 'const'], 'type': 'i2c_conf_t', 'name': 'i2c_config'}
    ctx['structs'] = i2cs
    print(template_struct.render(ctx))

    uarts = dtparser.render_peripherals('usart')
    ctx['type'] = 'uart_conf_t'
    ctx['name'] = 'uart_config'
    ctx['structs'] = uarts
    print(template_struct.render(ctx))


    # ctx = {}
    # for el in q:
    #     # remove None values so they don't get printed
    #     if el['F'] is None:
    #         del el['F']

    #     if not ctx.get(el['group']):
    #         # Connector numbering starts with 1 in boards, so we add padding
    #         ctx[el['group']] = [None, el]
    #     else:
    #         ctx[el['group']].append(el)

    # template_pinout = os.environ['TEMPLATE_PINOUT']
    # template_pinout_dir = os.path.dirname(template_pinout)
    # template_pinout_base = os.path.basename(template_pinout)

    # env = Environment(
    #     loader=FileSystemLoader([template_pinout_dir]),
    #     lstrip_blocks=True,
    #     trim_blocks=True
    # )

    # template_board = env.get_template(template_pinout_base)
    # #board['board']['cpu_model'] = cpu['information']['model'].upper()
    # ctx['board'] = board
    # #print(template_board.render(ctx))

    # file = open(template_pinout, 'w')
    # file.write(template_board.render(ctx))
    # file.close()
