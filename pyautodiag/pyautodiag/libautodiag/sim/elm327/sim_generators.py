import os
from ctypes import *
from ...libloader import load_lib

char_p = c_char_p
c_void_p = c_void_p

lib = load_lib()

CALLBACK_TYPE = CFUNCTYPE(None, c_void_p, POINTER(char_p), c_void_p, c_void_p)

class _ECUEmulationGenerator(Structure):
    _fields_ = [
        ("context", c_void_p),
        ("obd_sim_response", CALLBACK_TYPE),
        ("type", char_p)
    ]

lib.sim_ecu_generator_new_random.restype = POINTER(_ECUEmulationGenerator)
lib.sim_ecu_generator_new_cycle.restype = POINTER(_ECUEmulationGenerator)

class ECUEmulationGeneratorRandom(_ECUEmulationGenerator):
    def __new__(cls):
        ptr = lib.sim_ecu_generator_new_random()
        if not ptr:
            raise MemoryError("Failed to create random generator")
        obj = ptr.contents
        obj.__class__ = cls
        return obj

class ECUEmulationGeneratorCycle(_ECUEmulationGenerator):
    def __new__(cls):
        ptr = lib.sim_ecu_generator_new_cycle()
        if not ptr:
            raise MemoryError("Failed to create cycle generator")
        obj = ptr.contents
        obj.__class__ = cls
        return obj