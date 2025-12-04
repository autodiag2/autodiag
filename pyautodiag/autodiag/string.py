from autodiag.libloader import *

class object_string(Structure):
    _fields_ = [
        ("data", c_char_p)
    ]
    def __new__(cls):
        lib.object_string_new.restype = POINTER(cls)
        ptr = lib.object_string_new()
        if not ptr:
            raise MemoryError("Failed to create Serial instance")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj

class list_object_string(Structure):
    _fields_ = [
        ("size", c_int),
        ("list", POINTER(POINTER(object_string))),
    ]
    def __new__(cls):
        lib.list_object_string_new.restype = POINTER(cls)
        ptr = lib.list_object_string_new()
        if not ptr:
            raise MemoryError("Failed to create Serial instance")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj

    def append(self, o: object_string): return lib.list_object_string_append(byref(self), byref(o))
    def remove_at(self, i: int): return lib.list_object_string_remove_at(byref(self), i)
