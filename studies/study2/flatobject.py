import numpy as np

class Buffer(object):
    value_t = np.dtype({'names'  :['f64','i64','u64'],
                        'formats':['<f8','<i8','<u8'],
                        'offsets':[    0,    0,    0],
                        'itemsize':8})
    def __init__(self, initial_size=128):
        self.size= initial_size
        self.data=np.zeros(self.size,dtype=Buffer.value_t)
        self.data[0]=self.data.ctypes.data
        self.last=2
        self.data[1]=self.last
        self.objects=[]
        self.pointers=[]
    def allocate(self,size):
        last=self.last
        self.last+=size
        if self.last>self.size:
            self.size*=2
            olddata=self.data
            self.data=np.zeros(self.size,dtype=Buffer.value_t)
            self.data[:last]=olddata[:last]
        self.objects.append(last)
        self.data[1]=self.last
        return last
    def add_pointers(self,plist):
        self.pointers.extend(plist)

class Field(object):
    def __init__(self,datatype=None, value=None, const=False, length=None):
        if datatype is not None:
            self.datatype=datatype
        self.const=const
        self.value=value
        if value is not None:
            self.const=True
        self.length=length
        self.index=None
        self.name=None
    def _get_length(self,nvargs):
        if self.length is None:
            return 1
        elif isinstance(self.length, str):
            return eval(self.length,{},nvargs)
        else:
            return self.length
        return eval(self.length,{},nvargs)
    def _get_size(self,nvargs):
        if hasattr(self.datatype,'_get_size'):
            return self._get_size(nvargs)
        else:
            return self.itemsize*self.length
    def __get__(self,obj, type=None):
        if obj is None :
            return self
        offset=obj._offset[self.index]
        if self.size is None:
            return obj._data.data[self.datatype][offset]
        else:
            size  =obj._size[self.index]
            return obj._data.data[self.datatype][offset:offset+size]
    def __set__(self,obj, value):
        if not self.const or hasattr(obj,'_init'):
            offset=obj._offset[self.index]
            if self.size is None:
                obj._data.data[self.datatype][offset]=value
            else:
                size  =obj._size[self.index]
                obj._data.data[self.datatype][offset:offset+size]=value
        else:
            raise AttributeError("Cannot set constant attribute `%s`"%self.name)


class u64(Field):
    itemsize=1
    datatype='u64'

class f64(Field):
    itemsize=1
    datatype='f64'

class i64(Field):
    itemsize=1
    datatype='i64'

class MetaObject(type):
    def __init__(self, name, bases, d):
        type.__init__(self, name, bases, d)
        fields=[]
        ik=0
        for k,v in d.items():
            if isinstance(v,Field):
                v.index=ik
                v.name=k
                ik+=1
                fields.append(v)
        self._fields=fields

class Object(object,metaclass=MetaObject):
    def __init__(self,data, **nvargs):
        self._data=data
        self._offset=[]
        self._size=[]
        # set offsets
        self.elemid=self._data.allocate(self._get_size(nvargs))
        for iv,v in enumerate(self._fields):
            self._offset[iv]+=self.elemid
        # set values
        self._init=True
        for iv,v in enumerate(self._fields):
            if v.value is not None:
                setattr(self,v.name,v.value)
        for k,v in nvargs.items():
            setattr(self,k,v)
        del self._init
    def _get_size(self,nvargs):
        size=0
        for v in self._fields:
            self._offset.append(size)
            vsize=v._get_size(nvargs)
            self._size.append(vsize)
            size+=vsize
        return size


