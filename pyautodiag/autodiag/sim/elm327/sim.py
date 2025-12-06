import os
from autodiag.libloader import *
from autodiag.sim.ecu.generator import Generator
from autodiag.buffer import Buffer

class SimELM327(Structure):
    pass

class SimECU(Structure):

    def __new__(cls, address: int):
        obj_ptr = lib.sim_ecu_emulation_new(byte(address))
        if not obj_ptr:
            raise MemoryError("Failed to create SimECU")
        obj = cast(obj_ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj
    
    def set_generator(self, generator: Generator):
        self.generator = cast(pointer(generator), POINTER(Generator))

SimECU.CALLBACK_SAEJ1979_SIM_RESPONSE = CFUNCTYPE(
    char_p,
    POINTER(SimELM327),
    POINTER(SimECU),
    POINTER(Buffer)
)

SimECU._fields_ = [
    ("address", byte),
    ("generator", POINTER(Generator)),
    ("sim_ecu_response", SimECU.CALLBACK_SAEJ1979_SIM_RESPONSE)
]

lib.sim_ecu_emulation_new.argtypes = [byte]
lib.sim_ecu_emulation_new.restype = POINTER(SimECU)

from autodiag.lib import *

class list_SimECU(Structure):
    _fields_ = [
        ("size", c_int),
        ("list", POINTER(POINTER(SimECU)))
    ]

    def __new__(cls):
        obj_ptr = lib.list_SimECU_new()
        if not obj_ptr:
            raise MemoryError("Failed to create SimECU")
        obj = cast(obj_ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj
    
    def append(self, element):
        lib.list_SimECU_append(byref(self), cast(byref(element), POINTER(SimECU)))
    
    def remove(self, element):
        lib.list_SimECU_remove(byref(self), cast(byref(element), POINTER(SimECU)))

    def remove_at(self, index):
        return lib.list_SimECU_remove_at(byref(self), index).contents

lib.list_SimECU_new.restype = POINTER(list_SimECU)
lib.list_SimECU_free.argtypes = [POINTER(list_SimECU)]
lib.list_SimECU_append.argtypes = [POINTER(list_SimECU), POINTER(SimECU)]
lib.list_SimECU_remove.argtypes = [POINTER(list_SimECU), POINTER(SimECU)]
lib.list_SimECU_remove.restype = bool
lib.list_SimECU_remove_at.argtypes = [POINTER(list_SimECU), c_int]
lib.list_SimECU_remove_at.restype = POINTER(SimECU)

