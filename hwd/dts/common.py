import logging
from peewee import *
from hwd.binding_index import import_bindings
import os
import fdt
import re
import sys
import ruamel.yaml as yaml


db = SqliteDatabase(':memory:')

_nodes = {}
def node(cls):
    """Specifies that a class is a DT binding.
    """
    _nodes[cls.get_node_name()] = cls
    return cls

class BaseModel(Model):
    class Meta:
        database = db

class MissingDBTAttributeException(Exception):
    pass

class InvalidCellReference(Exception):
    pass

class InvalidPeripheralStatus(Exception):
    """Exception raised when an invalid value is assigned to the status
    property of a peripheral in the Device Tree.
    """
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

        node_props = {}
        for k, v in dtb_node.props.items():
            nk = k.replace('-', '_').replace('#', '')
            node_props[nk] = v

        d = {"id": dtb_node.model_id, "node": dtb_node}
        d.update(node_props)

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
    @property
    def target(self):
        node = DTBNode.get(phandle=self.phandle)

        parent_obj = node.model
        return parent_obj


class NodeModel(BaseModel):
    _node_name = None
    id = CharField(primary_key=True)
    node = ForeignKeyField(DTBNode)

    class CellData:
        """Metadata on cells that point to a specific type of node.

        Properties:
            - cells: Array of names for each position in the cell
            - unique: Forces that only one reference to this type of node exists
        """
        cells = []
        unique = False

    @classmethod
    def cell_class(cls):
        cells = cls.CellData.cells

        d = {"parent_class": cls }
        d['phandle'] = IntegerField(unique=cls.CellData.unique)
        for c in cells:
            d[c] =  CellAttr()

        __cls = type("{}Cell".format(cls.__name__),
                (CellClass,), d)
        return __cls

    @property
    def cells(self):
        d = {}
        for k,v in self._meta.fields.items():
            if isinstance(v, PhandleTo):
                d[k] = getattr(self, k)
        return d

    @classmethod
    def get_node_name(cls):
        """Get the name of the node a binding represents in the Device Tree.

        The lowercase version of the class name should match the type of the
        node (e.g. Usart is the binding for usart@4000800 node).

        In the case a class wants to represent a node type different to its name,
        then the variable '_node_name' should contain the string of the node type
        (e.g. _node_name = 'interrupt-controller').
        """
        return cls._node_name.lower() if cls._node_name else cls.__name__.lower()

    def render(self):
        """
        Renders the node information into a dictionary containing every field
        that is needed in the initialization structure.
        """
        pass

class PhandleTo(ForeignKeyField):
    """Represents a phandle to a node.
    """
    def __init__(self, nodeType, *args, **kwargs):
        super().__init__(nodeType.cell_class(), *args, **kwargs)

    def db_value(self, value):
        if isinstance(value, int):
            value = [value]
        elif isinstance(value, str):
            value = [int(value)]

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
    """Base class for pin configuration bindings.
    """
    @staticmethod
    def render_pin(pin):
        """Render a pin to be used in RIOT.
        """
        if not pin:
            return 'GPIO_UNDEF'
        return 'GPIO_PIN({},{})'.format(pin.target.label, pin.num)

class String(CharField):
    """String node property type.
    """
    pass

class Number(IntegerField):
    """Integer number node property type.
    """
    pass

class PeripheralStatus(CharField):
    """Status of a given peripheral in the Device Tree.

    Possible values for this field are:
        - okay: Indicates the device is operational
        - disabled: Indicates that the device is not operational.
    """
    choices = ['okay', 'disabled']

    def db_value(self, value):
        if value not in PeripheralStatus.choices:
            raise InvalidPeripheralStatus('{} status is not valid.'.format(value))
        return value

class Peripheral(NodeModel):
    """MCU peripheral binding base class.
    """
    pass

class CpuPin(Model):
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

class Pinmap(Model):
    """Physical description of a pin in a board.

    Each pin will be part of a group (e.g. Connector 'CN1' or 'leds') and will
    have a label (preferably the one printed on the board).
    """

    id = AutoField()
    label = CharField()
    pin = CharField()
    group = CharField()
    cpu_pin = ForeignKeyField(CpuPin, null=True)

    @property
    def L(self):
        return self.label

    class Meta:
        database = db

class DuplicatePinException(Exception):
    pass

class Board:
    def __init__(self, dtb):
        """Loads the hardware description (Device Tree Blob) for a given board.

        Loads and parses a '.dtb' file (Device Tree Blob) that describes the
        board.

        :param str dtb:    Path to the file
        """
        dtb_data = open(dtb, 'rb').read()
        self.tree = fdt.parse_dtb(dtb_data)

        self._import_bindings()
        db.create_tables([DTBNode, DTBProp, DTBPropCell, CpuPin])

        for _, v in _nodes.items():
            db.create_tables([v.cell_class()])

        for p in _nodes.values():
            p.create_table()

        for w in self.tree.walk():
            DTBNode.create_node(w)

        for n in DTBNode.select().where(DTBNode.model_name != None):
                DTBNode.create_model(n)

        # once all nodes are loaded, get the CpuPin
        self._extract_cpupin()

    def _import_bindings(self):
        """ Try to get bindings for every node that declares compatible strings.
        """
        for compatible in self.tree.search('compatible'):
            for option in compatible:
                (vendor, model) = option.split(',')
                if import_bindings(vendor, model):
                    logging.info('Found binding for [{} - {}]'
                        .format(vendor,model))
                    break


    def load_description(self, yml):
        """Loads a board description in YML format.

        :param str yml: Path to the file
        """
        with open(yml) as stream:
            self.desc = yaml.safe_load(stream)

        db.create_tables([Pinmap])

        # create pinmap table
        try:
            for k, v in self.desc['board']['pinmap'].items():
                for el in v:
                    el['group'] = k
                    try:
                        el['cpu_pin'] = CpuPin.get(pin=el['pin'])
                        logging.debug('{} pin found'.format(el['pin']))
                    except:
                        el['cpu_pin'] = None
                        logging.debug('{} pin not found'.format(el['pin']))
                    Pinmap.create(**el)
        except KeyError:
            logging.info('No board pinmap provided')

    def get_chosen(self):
        """Returns the chosen models grouped by function.

        Inspects the 'chosen' node for models and returns a dictionary where
        each key is a functionality and the value is the array of chosen models
        for that functionality.

        :return:    Dictionary of models for each functionality.
        """
        ret = {}
        # get all chosen nodes
        for function, nodes in DTBNode.get(path='/chosen').props.items():
            group = function.split(',')[1]

            # when only one phandle is defined the library returns a string
            if type(nodes) is str:
                nodes = [nodes]

            models = []
            for n in nodes:
                models.append(DTBNode.get(phandle=n).model)
            ret[group] = models

        return ret

    def get_pinmap(self):
        if not self.desc:
            return None
        ret = {}
        for p in Pinmap.select():
            if not ret.get(p.group):
                ret[p.group] = [p]
            else:
                ret[p.group].append(p)
        return ret

    def get_description(self):
        """Get board description from YML board file.

        :return: Board description
        """
        ret = {}
        ret['name'] = self.desc['board']['name']
        ret['cpu'] = self.desc['board']['cpu']
        ret['desc'] = getattr(self.desc['board'], 'description', '')
        return ret

    def _extract_cpupin(self):
        for config_group, models in self.get_chosen().items():
            for i,model in enumerate(models):
                if model.status != 'okay':
                    logging.warning('{} number {} chosen but not enabled. To '
                    'use the peripheral set its status to \'okay\''
                    .format(model.get_node_name(), str(i)))

                if hasattr(model, 'pinctrl') and model.pinctrl:
                    for k,v in model.pinctrl.target.cells.items():
                        if not v:
                            continue
                        pin_label = "{}{}".format(v.target.label, v.num)
                        el = {'pin': pin_label, 'function': k,
                              'config_group': config_group + str(i)}
                        try:
                            CpuPin.create(**el)
                        except IntegrityError:
                            dp = CpuPin.get(pin=pin_label)
                            raise DuplicatePinException("Both {} and {} are"
                                "trying to assign the same pin: {}"
                                .format("{}.{}"
                                    .format(dp.config_group, dp.function), "{}.{}"
                                        .format(config_group, k), pin_label))

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
        def check_status(periph):
            status = None
            try:
                status = periph.status
            except:
                return False
            return status == 'okay'

        ret = []
        q = DTBNode.select()
        if type is not None:
            q = q.where(DTBNode.model_name == type)

        for n in q:
            if n.model and (only_active and check_status(n.model) or not only_active):
                ret.append(n.model.render())

        return ret
