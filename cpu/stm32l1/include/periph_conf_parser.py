import ruamel.yaml
import json
from jinja2 import Environment, FileSystemLoader, select_autoescape

yaml = ruamel.yaml.YAML()

env = Environment(
    loader=FileSystemLoader('.'),
    lstrip_blocks=True,
    trim_blocks=True
)

template_periph_conf = env.get_template('periph_conf.jinja')

with open('stm32l151x6.yml') as stream:
    merged = stream.read(-1)

with open('periph_conf.yml') as stream:
    merged += stream.read(-1)

periph_conf = yaml.load(merged)['periph_conf']

#print(template_periph_conf.render(conf=periph_conf))

print(json.dumps(periph_conf))
