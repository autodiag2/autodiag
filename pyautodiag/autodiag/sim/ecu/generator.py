from autodiag.libloader import *
from autodiag.buffer import Buffer

class Generator(Structure):
    pass

Generator.CALLBACK_SIM_ECU_RESPONSE = CFUNCTYPE(
    None,
    POINTER(Generator),
    POINTER(Buffer),
    POINTER(Buffer)
)

Generator._fields_ = [
    ("context", c_void_p),
    ("type", char_p),
    ("state", c_void_p),
    ("response_trashed", c_void_p),
    ("response", Generator.CALLBACK_SIM_ECU_RESPONSE),
]

class GeneratorRandom(Generator):
    def __new__(cls):
        lib.sim_ecu_generator_new_random.restype = POINTER(Generator)
        ptr = lib.sim_ecu_generator_new_random()
        if not ptr:
            raise MemoryError("Failed to create random generator")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj

class GeneratorCycle(Generator):
    def __new__(cls):
        lib.sim_ecu_generator_new_cycle.restype = POINTER(Generator)
        ptr = lib.sim_ecu_generator_new_cycle()
        if not ptr:
            raise MemoryError("Failed to create cycle generator")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj

class GeneratorCitroenC5X7(Generator):
    def __new__(cls):
        lib.sim_ecu_generator_new_citroen_c5_x7.restype = POINTER(Generator)
        ptr = lib.sim_ecu_generator_new_citroen_c5_x7()
        if not ptr:
            raise MemoryError("Failed to create cycle generator")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj
