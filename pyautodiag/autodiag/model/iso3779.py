from autodiag.libloader import *
from autodiag.buffer import Buffer

class ISO3779(Structure):
    
    _fields_ = [
        ("country", c_char_p),
        ("manufacturer", c_char_p),
        ("year", c_int),
        ("wmi", POINTER(c_ubyte)),
        ("vds", POINTER(c_ubyte)),
        ("vis", POINTER(c_ubyte)),
        ("vin", POINTER(Buffer))
    ]

    def __new__(cls, vin: Buffer):
        lib.ISO3779_new.argtypes = [POINTER(Buffer)]
        lib.ISO3779_new.restype = POINTER(ISO3779)
        ptr = lib.ISO3779_new(pointer(vin))
        if not ptr:
            raise MemoryError("Failed to create ISO3779 instance")
        obj = ptr.contents
        obj.__class__ = cls
        return obj
    
    def set(self, vin: Buffer):
        lib.ISO3779_set.argtypes = [POINTER(ISO3779), POINTER(Buffer)]
        lib.ISO3779_set(byref(self), byref(vin))
    
    def decode(self, year=None):
        if year is None:
            lib.ISO3779_decode.argtypes = [POINTER(ISO3779)]
            lib.ISO3779_decode(byref(self))
        else:
            lib.ISO3779_decode_at_year.argtypes = [POINTER(ISO3779), c_int]
            lib.ISO3779_decode_at_year(byref(self), year)
    
    def manufacturer_is_less_500(self):
        lib.ISO3779_manufacturer_is_less_500.argtypes = [POINTER(ISO3779)]
        lib.ISO3779_manufacturer_is_less_500.restype = c_bool
        return lib.ISO3779_manufacturer_is_less_500(byref(self))

    def region(self):
        lib.ISO3779_region.argtypes = [POINTER(ISO3779)]
        lib.ISO3779_region.restype = c_char_p
        return lib.ISO3779_region(byref(self))

    def dump(self):
        lib.ISO3779_dump.argtypes = [POINTER(ISO3779)]
        return lib.ISO3779_dump(byref(self))
