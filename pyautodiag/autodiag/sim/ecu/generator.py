from ctypes import POINTER, Structure, CFUNCTYPE, c_bool, c_char_p, c_void_p, c_ubyte

from autodiag.buffer import Buffer
from autodiag.libloader import *

class Generator(Structure):
    """
    Holds how the ECU react to data bytes (UDS, OBD, CAN messages)
    """
    pass

Generator.RESPONSE = CFUNCTYPE(
    None,
    POINTER(Generator),
    POINTER(Buffer),
    POINTER(Buffer)
)

GeneratorResponsePIDFunc = CFUNCTYPE(POINTER(Buffer), POINTER(Generator), c_ubyte, c_int)
GeneratorResponsePIDsFunc = CFUNCTYPE(POINTER(Buffer), POINTER(Generator), POINTER(Buffer))
GeneratorResponseDTCWrapperFunc = CFUNCTYPE(POINTER(Buffer), POINTER(Generator), c_int)
GeneratorResponseDTCFunc = CFUNCTYPE(POINTER(Buffer), POINTER(Generator), c_int)
GeneratorResponseVehicleInfoFunc = CFUNCTYPE(POINTER(Buffer), POINTER(Generator), POINTER(Buffer))
GeneratorResponseVehicleInfoTypeFunc = CFUNCTYPE(POINTER(Buffer), POINTER(Generator), c_ubyte)
GeneratorResponseFunc = CFUNCTYPE(POINTER(Buffer), POINTER(Generator), POINTER(Buffer))
GeneratorResponseForPythonFunc = CFUNCTYPE(None, POINTER(Generator), POINTER(Buffer), POINTER(Buffer))
GeneratorContextToStringFunc = CFUNCTYPE(c_char_p, POINTER(Generator))
GeneratorContextLoadFromStringFunc = CFUNCTYPE(c_bool, POINTER(Generator), c_char_p)
GeneratorResponseUDSDidWrapperFunc = CFUNCTYPE(POINTER(Buffer), POINTER(Generator), POINTER(Buffer))
GeneratorResponseUDSDidFunc = CFUNCTYPE(POINTER(Buffer), POINTER(Generator), c_int16)
class GeneratorFlavour(Structure):
    _fields_ = [
        ("is_Iso15765_4", c_bool),
    ]

Generator._fields_ = [
    ("context", c_void_p),
    ("type", c_char_p),
    ("state", c_void_p),
    ("flavour", GeneratorFlavour),
    ("response_saej1979_pid", GeneratorResponsePIDFunc),
    ("response_saej1979_pids", GeneratorResponsePIDsFunc),
    ("response_saej1979_dtcs_wrapper", GeneratorResponseDTCWrapperFunc),
    ("response_saej1979_dtcs", GeneratorResponseDTCFunc),
    ("response_saej1979_vehicle_identification_request", GeneratorResponseVehicleInfoFunc),
    ("response_saej1979_vehicle_identification_request_info_type", GeneratorResponseVehicleInfoTypeFunc),
    ("response_uds_did_wrapper", GeneratorResponseUDSDidWrapperFunc),
    ("response_uds_did", GeneratorResponseUDSDidFunc),
    ("response_unused", GeneratorResponseFunc),
    ("response", GeneratorResponseForPythonFunc),
    ("context_to_string", GeneratorContextToStringFunc),
    ("context_load_from_string", GeneratorContextLoadFromStringFunc),
]

lib.sim_ecu_generator_new_random.argtypes = []
lib.sim_ecu_generator_new_random.restype = POINTER(Generator)

lib.sim_ecu_generator_new_cycle.argtypes = []
lib.sim_ecu_generator_new_cycle.restype = POINTER(Generator)

lib.sim_ecu_generator_new_citroen_c5_x7.argtypes = []
lib.sim_ecu_generator_new_citroen_c5_x7.restype = POINTER(Generator)

lib.sim_ecu_generator_new.argtypes = []
lib.sim_ecu_generator_new.restype = POINTER(Generator)

lib.sim_ecu_generator_new_replay.argtypes = []
lib.sim_ecu_generator_new_replay.restype = POINTER(Generator)

class GeneratorRandom(Generator):
    def __new__(cls):
        ptr = lib.sim_ecu_generator_new_random()
        if not ptr:
            raise MemoryError("Failed to create random generator")
        return ptr.contents

class GeneratorCycle(Generator):
    def __new__(cls):
        ptr = lib.sim_ecu_generator_new_cycle()
        if not ptr:
            raise MemoryError("Failed to create cycle generator")
        return ptr.contents

class GeneratorCitroenC5X7(Generator):
    def __new__(cls):
        ptr = lib.sim_ecu_generator_new_citroen_c5_x7()
        if not ptr:
            raise MemoryError("Failed to create generator")
        return ptr.contents