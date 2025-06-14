import os
from autodiag.libloader import *
from autodiag.sim.elm327.sim_generators import ECUEmulationGenerator

class ELM327emulation(Structure):
    pass

CALLBACK_TYPE = CFUNCTYPE(char_p, POINTER(c_void_p), POINTER(ELM327emulation), char_p, bool)

class ECUEmulation(Structure):
    _fields_ = [
        ("address", byte),
        ("generator", POINTER(ECUEmulationGenerator)),
        ("saej1979_sim_response", CALLBACK_TYPE)
    ]

    def __new__(cls, address: int):
        obj_ptr = lib.ecu_emulation_new(byte(address))
        if not obj_ptr:
            raise MemoryError("Failed to create ECUEmulation")
        obj = cast(obj_ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj
    
lib.ecu_emulation_new.argtypes = [byte]
lib.ecu_emulation_new.restype = POINTER(ECUEmulation)

class ECUEmulation_list(Structure):
    _fields_ = [
        ("size", c_int),
        ("list", POINTER(ECUEmulation))
    ]

    def __new__(cls):
        obj_ptr = lib.ECUEmulation_list_new()
        if not obj_ptr:
            raise MemoryError("Failed to create ECUEmulation")
        obj = cast(obj_ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj
    
    def append(self, element):
        lib.ECUEmulation_list_append(byref(self), cast(byref(element), POINTER(ECUEmulation)))
    
    def remove(self, element):
        lib.ECUEmulation_list_remove(byref(self), cast(byref(element), POINTER(ECUEmulation)))

    def remove_at(self, index):
        return lib.ECUEmulation_list_remove_at(byref(self), index).contents

lib.ECUEmulation_list_new.restype = POINTER(ECUEmulation_list)
lib.ECUEmulation_list_free.argtypes = [POINTER(ECUEmulation_list)]
lib.ECUEmulation_list_append.argtypes = [POINTER(ECUEmulation_list), POINTER(ECUEmulation)]
lib.ECUEmulation_list_remove.argtypes = [POINTER(ECUEmulation_list), POINTER(ECUEmulation)]
lib.ECUEmulation_list_remove.restype = bool
lib.ECUEmulation_list_remove_at.argtypes = [POINTER(ECUEmulation_list), c_int]
lib.ECUEmulation_list_remove_at.restype = POINTER(ECUEmulation)

