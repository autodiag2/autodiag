from autodiag.libloader import *

CALLBACK_TYPE = CFUNCTYPE(None, c_void_p, POINTER(char_p), c_void_p, c_void_p)

class ECUEmulationGenerator(Structure):
    _fields_ = [
        ("context", c_void_p),
        ("obd_sim_response", CALLBACK_TYPE),
        ("type", char_p)
    ]

    def __new__(cls, gen_type="random"):
        ptr = lib.sim_ecu_generator_new()
        if not ptr:
            raise MemoryError("Failed to create random generator")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj

    def __init__(self, gen_type="random"):
        self.type = gen_type.encode()

lib.sim_ecu_generator_new.restype = POINTER(ECUEmulationGenerator)
lib.sim_ecu_generator_new_random.restype = POINTER(ECUEmulationGenerator)
lib.sim_ecu_generator_new_cycle.restype = POINTER(ECUEmulationGenerator)

class ECUEmulationGeneratorRandom(ECUEmulationGenerator):
    def __new__(cls):
        ptr = lib.sim_ecu_generator_new_random()
        if not ptr:
            raise MemoryError("Failed to create random generator")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj

class ECUEmulationGeneratorCycle(ECUEmulationGenerator):
    def __new__(cls):
        ptr = lib.sim_ecu_generator_new_cycle()
        if not ptr:
            raise MemoryError("Failed to create cycle generator")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj