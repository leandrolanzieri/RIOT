import os
import json
from jinja2 import Environment, FileSystemLoader, select_autoescape
import yaml
import importlib
from peewee import *
import peewee
import inspect
from dts import base

def create_tables():
    base.CpuPin.bulk_from_dict(cpu['pins'])
    base.Pinmap.bulk_from_dict(board['board']['pinmap'])

    cpu_mod.Usart.bulk_from_dict(cpu['peripherals']['usart'])
    cpu_mod.Spi.bulk_from_dict(cpu['peripherals']['spi'])
    cpu_mod.I2C.bulk_from_dict(cpu['peripherals']['i2c'])
    cpu_mod.I2CPinmux.bulk_from_dict(cpu['peripherals']['pinmuxes']['i2c_pinmux'])
    cpu_mod.SpiPinmux.bulk_from_dict(cpu['peripherals']['pinmuxes']['spi_pinmux'])
    cpu_mod.UsartPinmux.bulk_from_dict(cpu['peripherals']['pinmuxes']['usart_pinmux'])
    cpu_mod.I2CConfig.bulk_from_dict(board['board']['periph_conf']['i2c'])
    cpu_mod.SpiConfig.bulk_from_dict(board['board']['periph_conf']['spi'])
    cpu_mod.UsartConfig.bulk_from_dict(board['board']['periph_conf']['uart'])

def format_function(function, config_group):
    return config_group + '.' + function


if __name__ == "__main__":

    env = Environment(
        loader=FileSystemLoader([os.environ['RIOTBASE'] + 'hwd/dts/', os.environ['RIOTBOARD']]),
        lstrip_blocks=True,
        trim_blocks=True
    )

    with open(os.environ['RIOTBOARD'] + 'board.yml') as stream:
        board = yaml.safe_load(stream)

    cpu_mod = importlib.import_module("dts."+board["board"]["cpu"].replace(".yml","").replace("/","."))

    template_cpu = env.get_template(board['board']['cpu'])
    cpu = yaml.safe_load(template_cpu.render())['cpu']
    create_tables()

    used_pins = []
    for m in inspect.getmembers(cpu_mod):
        if inspect.isclass(m[1]) and issubclass(m[1], base.Config):
            for conf in m[1].select():
                used_pins.extend(conf.get_pins())

    base.Pinout.bulk_from_dict(used_pins)

    q = base.Pinmap.select(base.Pinmap.label.alias('L'), base.Pinmap.pin, base.Pinmap.connector,
                      format_function(base.Pinout.function, base.Pinout.config_group) \
                      .alias('F')) \
              .join(base.Pinout, JOIN.LEFT_OUTER, on=(base.Pinmap.pin == base.Pinout.pin)).dicts()

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
