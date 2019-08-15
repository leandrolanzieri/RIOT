from peewee import *
import os
import fdt
import re
import hwd
import importlib

db = SqliteDatabase(':memory:')

_nodes = {}
def node(cells=[]):
    def inner(cls):
        cls.CELLS=cells
        _nodes[cls.__name__.lower()] = cls
        return cls
    return inner

class BaseModel(Model):
    class Meta:
        database = db

class CellAttr(IntegerField):
    pass

class CellClass(BaseModel):
    phandle = IntegerField()

    def resolve_cell(self):
        parent_obj = self.parent_class.get(phandle=self.phandle)

        d = {}
        for k, p in parent_obj._meta.fields.items():
            d[k] = getattr(parent_obj, k)

        for k, p in self._meta.fields.items():
            if isinstance(p, CellAttr):
                d[k] = getattr(self, k)

    @property
    def target(self):
        parent_obj = self.parent_class.get(phandle=self.phandle)
        return parent_obj


class NodeModel(BaseModel):
    id = CharField(primary_key=True)
    phandle = IntegerField(null=True)

    @classmethod
    def cell_class(cls):
        cells = cls.CELLS

        d = {"parent_class": cls }
        for c in cells:
            d[c] =  CellAttr()

        __cls = type("{}Cell".format(cls.__name__),
                (CellClass,), d)
        return __cls

    @property
    def cells(self):
        d = {}
        for k,v in self._meta.fields.items():
            if isinstance(v, Cell):
                d[k] = getattr(self, k)
        return d


class CellArray(BaseModel):
    parent_phandle = IntegerField()

    def get_values(self):
        cell_array = self
        d = {}
        for c in CellEntry.select().where(CellEntry.cell_array==cell_array):
            d[c.key] = c.value
        model_phandle = cell_array.parent_phandle
        rel_model = self.rel_model

        model = rel_model.get(phandle=model_phandle)
        d["node"] = model
        return d

class CellEntry(BaseModel):
    cell_array = ForeignKeyField(CellArray)
    key = CharField()
    value = IntegerField()

class Cell(ForeignKeyField):
    def db_value(self, value):
        if isinstance(value, int):
            value = [value]

        phandle = value[0]
        parent_model = self.rel_model.parent_class
        cells = parent_model.CELLS
        args = dict(zip(cells, value[1:]))
        return super().db_value(self.rel_model.create(phandle=phandle, **args).id)

class Pinctrl(NodeModel):
    @property
    def pins(self):
        return [(k,v) for k,v in self._meta.fields.items() if isinstance(v, Cell)]

class DTBTree(BaseModel):
    path = CharField()
    key = CharField()
    value = CharField(null=True)

class DTBParser:
    def __init__(self, cpu_name):
        self.bindings = self.get_cpu_bindings(cpu_name)
        self.tree = None

    def load(self, filename):
        dtb_data = open(os.environ['DTB'], 'rb').read()
        self.tree = fdt.parse_dtb(dtb_data)
        db.create_tables([CellArray, CellEntry])
        phandles = self.tree.search('phandle')

        db.create_tables([DTBTree])

        for p in _nodes.values():
            p.create_table()

        for w in self.tree.walk():
            path = w[0]
            for p in self.tree.get_node(path).props:
                key = p.name
                if hasattr(p, 'data'):
                    value = p.data[0] if len(p.data) == 1 else p.data
                else:
                    value = None
                DTBTree.create(path=path, key=key, value=value)

        for w in self.tree.walk():
            path = w[0]
            self.get_model(path)

    def get_cpu_bindings(self, cpu_name):
        for root, subdirs, files in os.walk(os.path.dirname(hwd.__file__)):
            if cpu_name in subdirs:
                module_name = "hwd.{}.bindings".format(os.path.relpath(os.path.join(root, cpu_name), start=os.path.dirname(hwd.__file__)).replace("/","."))
                module = importlib.import_module(module_name)
                return module
        return None


    def get_model(self, path):
        m = re.search("\/([\w]*)@(.*)$", path)
        if not m:
            return None

        model_type = m[1]
        model_id = m[2]
        model_class = _nodes.get(model_type, None)

        if not model_class:
            return None

        # Check if model exists. Create if not
        if(len(model_class.select().where(model_class.id == model_id)) > 0):
            return model_class.get(model_class.id == model_id)

        node = {"id": model_id}

        for p in self.tree.get_node(path).props:
            if hasattr(p, 'data'):
                node[p.name] = p.data[0] if len(p.data) == 1 else p.data
            else:
                node[p.name] = True

        return model_class.create(**node)


