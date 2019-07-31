import ruamel.yaml
import json
from jinja2 import Environment, FileSystemLoader, select_autoescape

yaml = ruamel.yaml.YAML()

env = Environment(
    loader=FileSystemLoader('.'),
    lstrip_blocks=True,
    trim_blocks=True,
    autoescape=select_autoescape(['xml', 'html'])
)

template_board = env.get_template('Nucleo-64_nv_plain.svg')

with open('stm32l151x6.yml') as stream:
    merged = stream.read(-1)

with open('periph_conf.yml') as stream:
    merged += stream.read(-1)

board = yaml.load(merged)['board']
periph_conf = board['periph_conf']

used_pins = []

def get_pins_of(dev, periph_conf):
    pins = []
    for idx, d in enumerate(periph_conf[dev]):
        pinmux = d.get('pinmux')
        if pinmux:
            for k,v in pinmux.items():
                if k.endswith('_pin'):
                    pins.append({'pin': v, 'name': dev + str(idx) + '.' + k.split('_pin')[0], 'cpu': 'P' + v[0] + str(v[1])})
    return pins

for k in periph_conf.keys():
    used_pins.extend(get_pins_of(k, periph_conf))

for pin in used_pins:
    if pin['pin']:
        board['P' + pin['pin'][0] + str(pin['pin'][1])] = pin['name']

board_pins = board['pin_map']
ctx = {'board': board}
ctx.update(board_pins)
print(template_board.render(ctx))
