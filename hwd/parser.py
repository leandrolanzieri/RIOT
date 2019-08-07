import os
import json
from jinja2 import Environment, FileSystemLoader, select_autoescape
import yaml
from dts.stm32.stm32l1.stm32l152re.stm32l152re import *
from lwd import *

def create_tables():
    Usart.setup()
    I2C.setup()
    Spi.setup()
    UsartPinmux.setup()
    I2CPinmux.setup()
    SpiPinmux.setup()
    UsartConfig.setup()
    I2CConfig.setup()
    SpiConfig.setup()
    Dma.setup()
    CpuPin.setup()
    Pinout.setup()
    Pinmap.setup()


def format_function(function, config_group):
    return config_group + '.' + function

def create_elements(cpu, board):
    CpuPin.bulk_from_dict(cpu['pins'])
    Usart.bulk_from_dict(cpu['peripherals']['usart'])
    Spi.bulk_from_dict(cpu['peripherals']['spi'])
    I2C.bulk_from_dict(cpu['peripherals']['i2c'])
    I2CPinmux.bulk_from_dict(cpu['peripherals']['pinmuxes']['i2c_pinmux'])
    SpiPinmux.bulk_from_dict(cpu['peripherals']['pinmuxes']['spi_pinmux'])
    UsartPinmux.bulk_from_dict(cpu['peripherals']['pinmuxes']['usart_pinmux'])
    I2CConfig.bulk_from_dict(board['board']['periph_conf']['i2c'])
    SpiConfig.bulk_from_dict(board['board']['periph_conf']['spi'])
    UsartConfig.bulk_from_dict(board['board']['periph_conf']['uart'])
    Pinmap.bulk_from_dict(board['board']['pinmap'])


if __name__ == "__main__":

    env = Environment(
        loader=FileSystemLoader([os.environ['RIOTBASE'] + 'hwd/dts/', os.environ['RIOTBOARD']]),
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
