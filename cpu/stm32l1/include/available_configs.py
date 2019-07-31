import ruamel.yaml
import json

yaml = ruamel.yaml.YAML()

with open('stm32l151x6.yml') as stream:
    cpu = yaml.load(stream)

peripherals = {}

for p in cpu['peripherals']:
    pinmuxes = []
    for m in cpu['pinmux_table']:
        if m['periph']['_label'] == p['_label']:
            pinmuxes.append(m['pinmux'])
    peripherals[p['_label']] = {'peripheral': p, 'pinmuxes': pinmuxes}


print('== Available Configurations ==')
for k, p in peripherals.items():
    print('- Device type:{}'.format(p['peripheral']['type']))
    print('- Label:{}'.format(p['peripheral']['_label']))
    print('-- Pinmux configurations --')
    for pinmux in p['pinmuxes']:
        print('- Label:{}'.format(pinmux['_label']))
    print('----\n')

#print(json.dumps(peripherals))