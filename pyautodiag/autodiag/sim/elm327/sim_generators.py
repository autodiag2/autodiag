from autodiag.libloader import *
from autodiag.buffer import Buffer

class ECUEmulationGenerator(Structure):

    def __new__(cls, gen_type="random"):
        ptr = lib.sim_ecu_generator_new()
        if not ptr:
            raise MemoryError("Failed to create random generator")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj

    def __init__(self, gen_type="random"):
        self.type = gen_type.encode()

ECUEmulationGenerator.CALLBACK_OBD_SIM_RESPONSE = CFUNCTYPE(
    None,
    POINTER(ECUEmulationGenerator),
    POINTER(char_p),
    POINTER(Buffer),
    POINTER(Buffer)
)

ECUEmulationGenerator._fields_ = [
    ("context", c_void_p),
    ("obd_sim_response", ECUEmulationGenerator.CALLBACK_OBD_SIM_RESPONSE),
    ("type", char_p)
]

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