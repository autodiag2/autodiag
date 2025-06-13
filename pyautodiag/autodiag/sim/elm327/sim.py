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
lib.ecu_emulation_new.restype = POINTER(ECUEmulation)  # raw pointer

class ECUEmulation_list(Structure):
    _fields_ = [
        ("size", c_int),
        ("list", POINTER(ECUEmulation))
    ]

