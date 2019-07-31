import yaml
from jinja2 import Environment, FileSystemLoader

env = Environment(
    loader=FileSystemLoader('.'),
    lstrip_blocks=True,
    trim_blocks=True
)

template = env.get_template('stm32.jinja')

with open('stm32l151x6.yml') as stream:
    try:
        af = yaml.safe_load(stream)['pinmux_table']
        print(template.render(af=af))

    except yaml.YAMLError as exc:
        print(exc)
