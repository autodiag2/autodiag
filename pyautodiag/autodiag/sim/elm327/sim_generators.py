from autodiag.libloader import *
from autodiag.buffer import Buffer

class SimECUGenerator(Structure):
    pass

SimECUGenerator.CALLBACK_SIM_ECU_RESPONSE = CFUNCTYPE(
    c_void_p,
    POINTER(SimECUGenerator),
    POINTER(Buffer)
)

SimECUGenerator._fields_ = [
    ("context", c_void_p),
    ("type", char_p),
    ("response", SimECUGenerator.CALLBACK_SIM_ECU_RESPONSE)
]

class SimECUGeneratorRandom(SimECUGenerator):
    def __new__(cls):
        lib.sim_ecu_generator_new_random.restype = POINTER(SimECUGenerator)
        ptr = lib.sim_ecu_generator_new_random()
        if not ptr:
            raise MemoryError("Failed to create random generator")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj

class SimECUGeneratorCycle(SimECUGenerator):
    def __new__(cls):
        lib.sim_ecu_generator_new_cycle.restype = POINTER(SimECUGenerator)
        ptr = lib.sim_ecu_generator_new_cycle()
        if not ptr:
            raise MemoryError("Failed to create cycle generator")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj

class SimECUGeneratorCitroenC5X7(SimECUGenerator):
    def __new__(cls):
        lib.sim_ecu_generator_new_citroen_c5_x7.restype = POINTER(SimECUGenerator)
        ptr = lib.sim_ecu_generator_new_citroen_c5_x7()
        if not ptr:
            raise MemoryError("Failed to create cycle generator")
        obj = cast(ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj
