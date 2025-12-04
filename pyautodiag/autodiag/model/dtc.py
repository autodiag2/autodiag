from autodiag.libloader import *
from autodiag.model.vehicle import Vehicle
from autodiag.model.vehicle import ECU
from autodiag.string import *

DTC_DATA_SZ = 3

class DTC_DESCRIPTION(Structure):
    _fields_ = [
        ("reason", c_char_p),
        ("solution", c_char_p),
        ("vehicle", POINTER(Vehicle)),
    ]

    def __new__(cls):
        lib.dtc_description_new.restype = POINTER(cls)
        ptr = lib.dtc_description_new()
        if not ptr:
            raise MemoryError("Failed to create Serial instance")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj
    
    def dump(self): return lib.dtc_description_dump(byref(self))

lib.dtc_description_new.restype = POINTER(DTC_DESCRIPTION)
lib.dtc_description_free.argtypes = [POINTER(DTC_DESCRIPTION)]
lib.dtc_description_dump.argtypes = [POINTER(DTC_DESCRIPTION)]

class list_DTC_DESCRIPTION(Structure):
    _fields_ = [
        ("size", c_int),
        ("list", POINTER(DTC_DESCRIPTION)),
    ]
    def __new__(cls):
        lib.list_DTC_DESCRIPTION_new.restype = POINTER(cls)
        ptr = lib.list_DTC_DESCRIPTION_new()
        if not ptr:
            raise MemoryError("Failed to create Serial instance")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj
    
    def append(self, dtc_desc: DTC_DESCRIPTION): return lib.list_DTC_DESCRIPTION_append(byref(self), byref(dtc_desc))

lib.list_DTC_DESCRIPTION_new.restype = POINTER(list_DTC_DESCRIPTION)
lib.list_DTC_DESCRIPTION_free.argtypes = [POINTER(list_DTC_DESCRIPTION)]
lib.list_DTC_DESCRIPTION_append.argtypes = [POINTER(list_DTC_DESCRIPTION), POINTER(DTC_DESCRIPTION)]

class DTC(Structure):
    pass

DTC._fields_ = [
    ("data", c_ubyte * DTC_DATA_SZ),
    ("description", POINTER(list_DTC_DESCRIPTION)),
    ("to_string", CFUNCTYPE(c_char_p, POINTER(DTC))),
    ("ecu", POINTER(ECU)),
    ("detection_method", POINTER(list_object_string)),
]

class list_DTC(Structure):
    _fields_ = [
        ("size", c_int),
        ("list", POINTER(DTC)),
    ]

lib.list_DTC_append_list.argtypes = [POINTER(list_DTC), POINTER(list_DTC)]
lib.list_DTC_get.argtypes = [POINTER(list_DTC), c_char_p]
lib.list_DTC_get.restype = POINTER(DTC)

lib.dtc_desc_fill_from_codes_file.argtypes = [POINTER(DTC), POINTER(DTC_DESCRIPTION)]
lib.dtc_description_fetch_from_fs.argtypes = [POINTER(DTC), POINTER(Vehicle)]