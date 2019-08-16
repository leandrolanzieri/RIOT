from peewee import *
import os
import fdt
import re
import hwd
import importlib

db = SqliteDatabase(':memory:')

_nodes = {}
def node(cls):
    _nodes[cls.__name__.lower()] = cls
    return cls


class BaseModel(Model):
    class Meta:
        database = db

class DTBNode(BaseModel):
    phandle = IntegerField(null=True)
    path = CharField()
    model_name = CharField(null=True)
    model_id = CharField(null=True)

    @property
    def model_class(self):
        return _nodes.get(self.model_name, None)

    @property
    def model(self):
        if(self.model_class):
            return self.model_class.get(id=self.model_id)
        return None

    @staticmethod
    def create_from_node(node):
        path = node[0]
        m = re.search("\/([a-zA-Z0-9_-]*)@([\w]*)$", path)
        model_type = m and m[1]
        model_id = m and m[2]
        phandle = None

        for p in node[2]:
            if(p.name == "phandle"):
                phandle = p.data[0]
           
        dtb_node = DTBNode.create(phandle=phandle, path=path, model_name=model_type, model_id=model_id)

        if not m:
            return None

        model_class = _nodes.get(model_type, None)

        if not model_class:
            return None

        d = {"id": model_id, "node": dtb_node}

        for p in node[2]:
            if hasattr(p, 'data'):
                d[p.name] = p.data[0] if len(p.data) == 1 else p.data
            else:
                d[p.name] = True

        return model_class.create(**d)



class CellAttr(IntegerField):
    pass

class CellClass(BaseModel):
    phandle = IntegerField()

    @property
    def target(self):
        node = DTBNode.get(phandle=self.phandle)

        assert node.model_class == self.parent_class

        parent_obj = node.model
        return parent_obj


class NodeModel(BaseModel):
    id = CharField(primary_key=True)
    node = ForeignKeyField(DTBNode)

    class CellData:
        cells = []

    @classmethod
    def cell_class(cls):
        cells = cls.CellData.cells

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

class Cell(ForeignKeyField):
    def db_value(self, value):
        if isinstance(value, int):
            value = [value]

        phandle = value[0]
        parent_model = self.rel_model.parent_class
        cells = parent_model.CellData.cells
        args = dict(zip(cells, value[1:]))
        return super().db_value(self.rel_model.create(phandle=phandle, **args).id)

class Pinctrl(NodeModel):
    pass

class Pinmap(Model):
    id = AutoField()
    label = CharField()
    pin = CharField()
    group = CharField()

    @property
    def L(self):
        return self.label

    class Meta:
        database = db

class Pinout(Model):
    pin = CharField(primary_key=True)
    function = CharField()
    config_group = CharField()

    class Meta:
        database = db

    @classmethod
    def bulk_from_dict(cls, dictionary):
        cls._meta.database.create_tables([cls])
        for el in dictionary:
            cls.create(**el)

class DTBParser:
    def __init__(self, cpu_name):
        self.bindings = self.get_cpu_bindings(cpu_name)
        for n, v in _nodes.items():
            db.create_tables([v.cell_class()])

        self.tree = None

    def load(self, filename):
        dtb_data = open(os.environ['DTB'], 'rb').read()
        self.tree = fdt.parse_dtb(dtb_data)
        db.create_tables([DTBNode])
        phandles = self.tree.search('phandle')

        for p in _nodes.values():
            p.create_table()

        for w in self.tree.walk():
            DTBNode.create_from_node(w)


    def get_cpu_bindings(self, cpu_name):
        for root, subdirs, files in os.walk(os.path.dirname(hwd.__file__)):
            if cpu_name in subdirs:
                module_name = "hwd.{}.bindings".format(os.path.relpath(os.path.join(root, cpu_name), start=os.path.dirname(hwd.__file__)).replace("/","."))
                module = importlib.import_module(module_name)
                return module
        return None


    def gen_pinout(self):
        for p in self.tree.get_node("/chosen").props:
            config_group = p.name.split(",")[1].upper()
            model = DTBNode.get(phandle=p.data[0]).model
            if model.status != 'okay':
                raise NotImplementedError

            if hasattr(model, 'pinctrl'):
                for k,v in model.pinctrl.target.cells.items():
                    pin_label = "{}{}".format(v.target.label, v.num)
                    el = {'pin': pin_label, 'function': k, 'config_group': config_group}
                    Pinout.create(**el)




