import importlib

_map = {
    'st': {
        'stm32l1': 'hwd.dts.arm.stm32.stm32l1.bindings'
    },
    'atmel': {
        'samd21': 'hwd.dts.arm.sam.samd.samd21.bindings',
        'samr21': 'hwd.dts.arm.sam.samd.samd21.bindings'
    },
    'arm': {
        'v7m-nvic': 'hwd.dts.arm.v7m_nvic'
    }
}

def import_bindings(vendor, model):
    """Imports the Device Tree bindings for a certain device.

    Inspects the available list of Python Device Tree bindings, if available the
    module is imported.

    :param str vendor: Vendor
    :param str model: Model of the device
    :return: Imported module if found
    :rtype: Module

    """
    cpus = _map.get(vendor)
    if cpus:
        path = cpus.get(model)
        if path:
            module = importlib.import_module(path)
            return module
    return None
