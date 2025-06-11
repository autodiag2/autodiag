import os
from ctypes import *
from autodiag.libautodiag.libloader import load_lib

byte = c_ubyte
bool = c_bool
char_p = c_char_p

class _ELM327emulation(Structure):
    pass

CALLBACK_TYPE = CFUNCTYPE(char_p, POINTER(c_void_p), POINTER(_ELM327emulation), char_p, bool)

lib = load_lib()
lib.ecu_emulation_new.argtypes = [byte]
lib.ecu_emulation_new.restype = POINTER(c_void_p)  # raw pointer

class ECUEmulation(Structure):
    _fields_ = [
        ("address", byte),
        ("generator", c_void_p),
        ("saej1979_sim_response", CALLBACK_TYPE)
    ]

    def __new__(cls, address: int):
        obj_ptr = lib.ecu_emulation_new(byte(address))
        if not obj_ptr:
            raise MemoryError("Failed to create ECUEmulation")
        obj = cast(obj_ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj