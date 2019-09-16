import logging
import os
import argparse
from jinja2 import Environment, FileSystemLoader, select_autoescape
from hwd.dts.common import Board

jinja_env = Environment(
    loader=FileSystemLoader([os.environ['RIOTBASE'], os.environ['DTS_TEMPLATES']]),
    lstrip_blocks=True,
    trim_blocks=True
)

def generate_periph_conf(board):
    ret = ''
    template_struct = jinja_env.get_template('array_of_structs.jinja')

    i2cs = board.render_peripherals('i2c')
    ctx = {'attributes': ['static', 'const'], 'type': 'i2c_conf_t', 'name': 'i2c_config'}
    ctx['structs'] = i2cs
    ret += template_struct.render(ctx) + '\n\n'

    uarts = board.render_peripherals('usart')
    ctx['type'] = 'uart_conf_t'
    ctx['name'] = 'uart_config'
    ctx['structs'] = uarts
    ret += template_struct.render(ctx) + '\n\n'

    spis = board.render_peripherals('spi')
    ctx['type'] = 'spi_conf_t'
    ctx['name'] = 'spi_config'
    ctx['structs'] = spis
    ret += template_struct.render(ctx)

    return ret

def generate_pinout(board, template_path):
    ctx = board.get_description()
    ctx['pins'] = {}
    for name, connector in board.get_pinmap().items():
        conn = [{}] # connector numeration starts in 1
        for pin in connector:
            if pin.cpu_pin:
                cg = pin.cpu_pin.config_group
                fu = pin.cpu_pin.function
            else:
                cg = ''
                fu = ''
            conn.append({'L':pin.label, 'F': '{}.{}'.format(cg,fu)})
        ctx['pins'][name] = conn
    try:
        template = jinja_env.get_template(template_path)
    except:
        template = jinja_env.get_template('board_pinout.jinja')

    return template.render(ctx)

def output(out, file=None):
    """Outputs a string to a file or STDOUT.

    :param str out:     String to output
    :param str file:    Path to the output file
    """
    try:
        with open(file, 'w') as f:
            f.write(out)
            f.close()
    except:
        print(out)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
            description='Perform operations on Device Trees and Bindings')

    parser.add_argument('dtb', help='Path to the .dtb file')
    parser.add_argument('cmd', help='Command')
    parser.add_argument('-b', '--board', help='Board description file')
    parser.add_argument('-p', '--pinout', help='Board pinout template')
    parser.add_argument('-o', '--output', help='Output file')
    parser.add_argument('--log', help='Logging level. Default: Warning.')
    args = parser.parse_args()

    # verify that the provided log level is valid
    if args.log:
        numeric_level = getattr(logging, args.log.upper(), None)
        if not isinstance(numeric_level, int):
            raise ValueError('Invalid log level: %s' % args.log)
        logging.basicConfig(level=numeric_level)

    board = Board(args.dtb)
    if args.board:
        board.load_description(args.board)

    if args.cmd == 'generate':
        output(generate_periph_conf(board), args.output)
    elif args.cmd == 'pinout':
        output(generate_pinout(board, args.pinout), args.output)
    else:
        raise NotADirectoryError
