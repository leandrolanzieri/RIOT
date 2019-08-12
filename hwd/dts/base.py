from peewee import *
import peewee
db = SqliteDatabase(':memory:')

class BaseModel(Model):
    id = CharField(primary_key=True)
    class Meta:
        database = db

    @classmethod
    def create(cls, *args, **kwargs):
        o = cls(**kwargs)
        if "id" not in kwargs:
            o.id = o.get_label()
        o.save(force_insert=True)
    
    @classmethod
    def bulk_from_dict(cls, dictionary):
        cls._meta.database.create_tables([cls])
        for el in dictionary:
            cls.create(**el)

class AlternateFunction(CharField):
    pass

class Periph(ForeignKeyField):
    pass

class Config(BaseModel):
    pass

class Macro(CharField):
    pass

class Pinmux(ForeignKeyField):
    pass

class CpuPin(BaseModel):
    port = CharField()
    num = IntegerField()

    def get_label(self):
        return "P{}{}".format(self.port, self.num)

class Pin(ForeignKeyField):
    def __init__(self, **kwargs):
        super().__init__(CpuPin, **kwargs)


class Pinout(Model):
    pin = Pin(primary_key=True)
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
    id = AutoField()
    label = CharField()
    pin = CharField()
    connector = CharField()

    @property
    def L(self):
        return self.label

    class Meta:
        database = db

    @classmethod
    def bulk_from_dict(cls, dictionary):
        cls._meta.database.create_tables([cls])
        for k, v in dictionary.items():
            for el in v:
                el['connector'] = k
                cls.create(**el)
class BasePinmux(BaseModel):
    def get_pins(self):
        l = []
        for k,v in self._meta.fields.items():
            if isinstance(v, Pin):
                l.append({'pin': getattr(self, k), 'function': k})
        return l

    def get_label(self):
        l = []
        string = str(self.periph)
        for k,v in self._meta.fields.items():
            if isinstance(v, Pin):
                l.append("{}_{}_{}".format(k, getattr(self, k).port, getattr(self, k).num).lower())
                
        l.sort()
        return "{}_{}".format(self.periph, "_".join(l))


class BasePinmuxConfig(Config):
    id = AutoField()
    class Meta:
        database = db

    @classmethod
    def create(cls, **kwargs):
        pinmux_class = cls.pinmux.rel_model
        pinmux_data = kwargs.pop("pinmux")

        # Check if pinmux exists. Create if not
        pinmux_id = pinmux_data.pop("id", None)

        if (pinmux_id):
            pinmux = pinmux_class.select().where(pinmux_class.id==pinmux_id).get()
            if pinmux_data:
                pinmux.update(**pinmux_data).execute()
        else:
            pinmux = pinmux_class(**pinmux_data)
            pinmux.id = pinmux.get_label()
            pinmux.save(force_insert=True)

        sp = cls(pinmux=pinmux, **kwargs)
        sp.save(force_insert=True)
        return sp

    @classmethod
    def bulk_from_dict(cls, dictionary):
        cls._meta.database.create_tables([cls])
        for el in dictionary:
            cls.create(**el)

    @property
    def config_group(self):
        return self.pinmux.periph.type + str(self.id - 1)

    def get_pins(self):
        l = []
        for k,v in self._meta.fields.items():
            if isinstance(v, Pin):
                l.extend([{'pin':self.cs, 'function':'cs'}])
            elif isinstance(v, Pinmux):
                l.extend(self.pinmux.get_pins())
        for p in l:
            p['config_group'] = self.config_group
        return l

def pinmux(periph):
    def inner(cls):
        setattr(cls, 'periph', Periph(periph))
        return cls
    return inner

