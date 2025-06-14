import os
from autodiag.libloader import *
from autodiag.sim.elm327.sim_generators import SimECUGenerator

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
    
    def set_generator(self, generator: SimECUGenerator):
        self.generator = cast(pointer(generator), POINTER(SimECUGenerator))

SimECU.CALLBACK_SAEJ1979_SIM_RESPONSE = CFUNCTYPE(
    char_p,
    POINTER(SimECU),
    POINTER(SimELM327),
    char_p,
    bool
)

SimECU._fields_ = [
    ("address", byte),
    ("generator", POINTER(SimECUGenerator)),
    ("sim_ecu_response", SimECU.CALLBACK_SAEJ1979_SIM_RESPONSE)
]

lib.sim_ecu_emulation_new.argtypes = [byte]
lib.sim_ecu_emulation_new.restype = POINTER(SimECU)

class SimECU_list(Structure):
    _fields_ = [
        ("size", c_int),
        ("list", POINTER(SimECU))
    ]

    def __new__(cls):
        obj_ptr = lib.SimECU_list_new()
        if not obj_ptr:
            raise MemoryError("Failed to create SimECU")
        obj = cast(obj_ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj
    
    def append(self, element):
        lib.SimECU_list_append(byref(self), cast(byref(element), POINTER(SimECU)))
    
    def remove(self, element):
        lib.SimECU_list_remove(byref(self), cast(byref(element), POINTER(SimECU)))

    def remove_at(self, index):
        return lib.SimECU_list_remove_at(byref(self), index).contents

lib.SimECU_list_new.restype = POINTER(SimECU_list)
lib.SimECU_list_free.argtypes = [POINTER(SimECU_list)]
lib.SimECU_list_append.argtypes = [POINTER(SimECU_list), POINTER(SimECU)]
lib.SimECU_list_remove.argtypes = [POINTER(SimECU_list), POINTER(SimECU)]
lib.SimECU_list_remove.restype = bool
lib.SimECU_list_remove_at.argtypes = [POINTER(SimECU_list), c_int]
lib.SimECU_list_remove_at.restype = POINTER(SimECU)

