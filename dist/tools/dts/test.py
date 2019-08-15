import os
import glob

import hwd
from hwd import dts
import importlib

def get_cpu_bindings(cpu_name):
    for root, subdirs, files in os.walk(os.path.dirname(hwd.__file__)):
        if cpu_name in subdirs:
            module_name = "hwd.{}.bindings".format(os.path.relpath(os.path.join(root, cpu_name), start=os.path.dirname(hwd.__file__)).replace("/","."))
            print(module_name)
            module = importlib.import_module(module_name)
            return module
    return None

module = get_cpu_bindings('stm32l152re')
print(dir(module))
