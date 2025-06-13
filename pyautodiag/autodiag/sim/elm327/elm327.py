from ctypes import *
from autodiag.libloader import load_lib
from autodiag.buffer import Buffer
from autodiag.sim.elm327.sim import ECUEmulation_list

byte = c_ubyte
bool = c_bool
char_p = c_char_p
c_double = c_double
c_int = c_int

lib = load_lib()

class ELM327emulationImplementation(Structure):
    pass

class CANSettings(Structure):
    _fields_ = [
        ("auto_format", bool),
        ("mask", POINTER(Buffer)),
        ("filter", POINTER(Buffer)),
        ("priority_29bits", byte),
        ("extended_addressing", bool),
        ("extended_addressing_target_address", byte),
        ("timeout_multiplier", c_int),
        ("display_dlc", bool),
    ]

class NVMSettings(Structure):
    _fields_ = [
        ("user_memory", byte),
        ("protocol", c_int),  # Assuming enum is mapped as int
        ("protocol_is_auto", bool),
        ("programmable_parameters", POINTER(Buffer)),
        ("programmable_parameters_pending", POINTER(Buffer)),
        ("programmable_parameters_states", POINTER(Buffer)),
    ]

class ELM327emulation(Structure):
    _fields_ = [
        ("implementation", POINTER(ELM327emulationImplementation)),
        ("eol", char_p),
        ("echo", bool),
        ("protocolRunning", c_int),
        ("protocol_is_auto_running", bool),
        ("printing_of_spaces", bool),
        ("printing_of_headers", bool),
        ("dev_description", char_p),
        ("dev_identifier", char_p),
        ("port_name", char_p),
        ("voltage", c_double),
        ("voltageFactory", c_double),
        ("baud_rate", c_int),
        ("baud_rate_timeout_msec", c_int),
        ("receive_address", POINTER(byte)),
        ("custom_header", POINTER(Buffer)),
        ("activity_monitor_count", byte),
        ("activity_monitor_timeout", c_int),
        ("vehicle_response_timeout", c_int),
        ("vehicle_response_timeout_adaptive", bool),
        ("obd_buffer", POINTER(Buffer)),
        ("responses", bool),
        ("programmable_parameters_defaults", POINTER(Buffer)),
        ("programmable_parameters_pending_load_type", POINTER(Buffer)),
        ("can", CANSettings),
        ("isMemoryEnabled", bool),
        ("testerAddress", byte),
        ("nvm", NVMSettings),
        ("ecus", POINTER(ECUEmulation_list))
    ]

    def __new__(cls):
        obj_ptr = lib.elm327_sim_new()
        if not obj_ptr:
            raise MemoryError("Failed to create ELM327 Emulation")
        obj = cast(obj_ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj

lib.elm327_sim_new.restype = POINTER(ELM327emulation)
lib.elm327_sim_loop.argtypes = [POINTER(ELM327emulation)]
lib.elm327_sim_loop_as_daemon.argtypes = [POINTER(ELM327emulation)]
lib.elm327_sim_destroy.argtypes = [POINTER(ELM327emulation)]
