import logging
from peewee import *
from hwd.binding_index import get_bindings
import os
import fdt
import re
import sys


db = SqliteDatabase(':memory:')

_nodes = {}
def node(cls):
    _nodes[cls.__name__.lower()] = cls
    return cls


class BaseModel(Model):
    class Meta:
        database = db

class MissingDBTAttributeException(Exception):
    pass

class InvalidCellReference(Exception):
    pass

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

    @property
    def props(self):
        d = {}
        for p in DTBProp.select().where(DTBProp.node==self):
            elems = DTBPropCell.select().where(DTBPropCell.prop==p)
            value = elems[0].value if len(elems)==1 else [el.value for el in elems]
            d[p.key] = value
        return d

    @staticmethod
    def create_node(node):
        path = node[0]
        m = re.search("\/([a-zA-Z0-9_-]*)@([\w]*)$", path)
        model_type = m and m[1]
        model_id = m and m[2]
        phandle = None

        for p in node[2]:
            if(p.name == "phandle"):
                phandle = p.data[0]
           
        dtb_node = DTBNode.create(phandle=phandle, path=path, model_name=model_type, model_id=model_id)


        for p in node[2]:
            prop = DTBProp.create(node=dtb_node, key=p.name)
            if hasattr(p, 'data'):
                for entry in p.data:
                    DTBPropCell.create(prop=prop, value=entry)



    @staticmethod
    def create_model(dtb_node):
        model_class = _nodes.get(dtb_node.model_name, None)

        if not model_class:
            return None

        d = {"id": dtb_node.model_id, "node": dtb_node}
        d.update(dtb_node.props)

        try:
            model_class.create(**d)
        except IntegrityError as e:
            exp = re.search("NOT NULL.*: ([\w]*\.[\w]*)$", e.args[0])
            raise MissingDBTAttributeException("{} not present in '{}'".format(exp[1], dtb_node.path))
        except InvalidCellReference as e:
            e.args= ["Failed creating {}: {}".format(dtb_node.path, e.args[0])]
            raise

class DTBProp(BaseModel):
    node = ForeignKeyField(DTBNode)
    key = CharField()

class DTBPropCell(BaseModel):
    prop = ForeignKeyField(DTBProp)
    value = CharField(null=True)

class CellAttr(IntegerField):
    pass

class CellClass(BaseModel):
    phandle = IntegerField()

    @property
    def target(self):
        node = DTBNode.get(phandle=self.phandle)

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

    def render(self):
        """
        Renders the node information into a dictionary containing every field
        that is needed in the initialization structure.
        """
        pass

class Cell(ForeignKeyField):
    def db_value(self, value):
        if isinstance(value, int):
            value = [value]

        phandle = value[0]
        parent_model = self.rel_model.parent_class
        pm = DTBNode.get(phandle=phandle).model_class
        if pm != parent_model:
            raise InvalidCellReference("Expected {} for {}, not {}".format(parent_model.__name__, self.name, pm.__name__))

        cells = parent_model.CellData.cells
        if(len(cells) != len(value[1:])):
            raise InvalidCellReference("{} expects {} cells, not {}".format(self.name, len(cells), len(value[1:])))
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

class DuplicatePinException(Exception):
    pass

class DTBParser:
    def load(self, filename):
        dtb_data = open(filename, 'rb').read()
        tree = fdt.parse_dtb(dtb_data)

        # try to get bindings for every node that declares compatible strings
        for compatible in tree.search('compatible'):
            for option in compatible:
                (vendor, model) = option.split(',')
                if get_bindings(vendor, model):
                    logging.info('Found binding for [{} - {}]'.format(vendor,model))
                    break

        db.create_tables([DTBNode, DTBProp, DTBPropCell])

        for _, v in _nodes.items():
            db.create_tables([v.cell_class()])

        for p in _nodes.values():
            p.create_table()

        for w in tree.walk():
            DTBNode.create_node(w)

        for n in DTBNode.select().where(DTBNode.model_name != None):
                DTBNode.create_model(n)

    def create_pinout(self):
        for k,v  in DTBNode.get(path="/chosen").props.items():
            config_group = k.split(",")[1].upper()
            model = DTBNode.get(phandle=v).model
            if model.status != 'okay':
                raise NotImplementedError

            if hasattr(model, 'pinctrl'):
                for k,v in model.pinctrl.target.cells.items():
                    if not v:
                        continue
                    pin_label = "{}{}".format(v.target.label, v.num)
                    el = {'pin': pin_label, 'function': k, 'config_group': config_group}
                    try:
                        Pinout.create(**el)
                    except IntegrityError:
                        dp = Pinout.get(pin=pin_label)
                        raise DuplicatePinException("Both {} and {} are trying to assign the same pin: {}".format("{}.{}".format(dp.config_group, dp.function), "{}.{}".format(config_group, k), pin_label))

    def render_peripherals(self, type=None, only_active=True):
        """Returns the information of peripherals.

        Renders the information and configuration of peripherals in a
        dictionary, that matches the needed fields for its configuration in
        RIOT.

        If no type is specify all peripherals are returned.

        :param str type:    Type of peripherals to render. If None all are rendered.
        :param bool only_active:    If True only peripherals with status 'okay' are rendered
        :return:    Array of dictionaries containing information and configuration of peripherals
        """
        ret = []
        q = DTBNode.select()
        if type is not None:
            q = q.where(DTBNode.model_name == type)

        for n in q:
            if n.model and (only_active and n.model.status == 'okay' or not only_active):
                ret.append(n.model.render())

        return ret
