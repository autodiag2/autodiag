from autodiag.libloader import *
from autodiag.buffer import Buffer

class SimECUGenerator(Structure):

    def __new__(cls, gen_type="random"):
        ptr = lib.sim_ecu_generator_new()
        if not ptr:
            raise MemoryError("Failed to create random generator")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj

    def __init__(self, gen_type="random"):
        self.type = gen_type.encode()

SimECUGenerator.CALLBACK_OBD_SIM_RESPONSE = CFUNCTYPE(
    None,
    POINTER(SimECUGenerator),
    POINTER(char_p),
    POINTER(Buffer),
    POINTER(Buffer)
)

SimECUGenerator._fields_ = [
    ("context", c_void_p),
    ("type", char_p),
    ("sim_ecu_generator_response", SimECUGenerator.CALLBACK_OBD_SIM_RESPONSE)
]

lib.sim_ecu_generator_new.restype = POINTER(SimECUGenerator)
lib.sim_ecu_generator_new_random.restype = POINTER(SimECUGenerator)
lib.sim_ecu_generator_new_cycle.restype = POINTER(SimECUGenerator)

class SimECUGeneratorRandom(SimECUGenerator):
    def __new__(cls):
        ptr = lib.sim_ecu_generator_new_random()
        if not ptr:
            raise MemoryError("Failed to create random generator")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj

class SimECUGeneratorCycle(SimECUGenerator):
    def __new__(cls):
        ptr = lib.sim_ecu_generator_new_cycle()
        if not ptr:
            raise MemoryError("Failed to create cycle generator")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj