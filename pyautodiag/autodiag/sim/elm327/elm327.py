from autodiag.libloader import *
from autodiag.buffer import Buffer
from autodiag.sim.elm327.sim import SimECU_list
from autodiag.lib import *

class SimELM327Implementation(Structure):
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

class SimELM327(Structure):
    _fields_ = [
        ("implementation", POINTER(SimELM327Implementation)),
        ("eol", char_p),
        ("echo", bool),
        ("protocolRunning", c_int),
        ("protocol_is_auto_running", c_int),
        ("printing_of_spaces", c_int),
        ("printing_of_headers", c_int),
        ("dev_description", char_p),
        ("dev_identifier", char_p),
        ("device_location", char_p),
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
        ("ecus", POINTER(SimECU_list))
    ]

    def __new__(cls):
        obj_ptr = lib.sim_elm327_new()
        if not obj_ptr:
            raise MemoryError("Failed to create ELM327 Emulation")
        obj = cast(obj_ptr, POINTER(cls)).contents
        obj.__class__ = cls
        return obj
    
    def loop(self, daemon=False):
        if daemon:
            lib.sim_elm327_loop_as_daemon(byref(self))
        else:
            lib.sim_elm327_loop(byref(self))
        
    def debug(self):
        lib.sim_elm327_debug(byref(self))

    def debug_from_python(self):
        print("SimELM327: {")
        if not self.ecus:
            print("    ecus: NULL")
            print("}")
            return
        ecu_list = self.ecus.contents
        print(f"    ecus {addr(self.ecus)} (list: {addr(ecu_list.list)}, size: {ecu_list.size}): {{")
        for i in range(ecu_list.size):
            sim_ecu = ecu_list.list[i]
            print(f"        ecu: {addr(pointer(sim_ecu))} {{")
            print(f"            address: {sim_ecu.address:02X}")
            generator = sim_ecu.generator.contents if sim_ecu.generator else None
            print(f"            generator: {addr(sim_ecu.generator)} {{")
            if generator:
                print(f"                context: {addr(generator.context)}")
                print(f"                type: {addr(generator.type)} {generator.type.decode() if generator.type else 'NULL'}")
                print(f"                sim_ecu_generator_response: {addr(generator.sim_ecu_generator_response)}")
            else:
                print("                NULL")
            print("            }")
            print(f"            sim_ecu_response: {addr(sim_ecu.sim_ecu_response)}")
            print("        }")
        print("    }")
        print("}")


lib.sim_elm327_new.restype = POINTER(SimELM327)
lib.sim_elm327_loop.argtypes = [POINTER(SimELM327)]
lib.sim_elm327_loop_as_daemon.argtypes = [POINTER(SimELM327)]
lib.sim_elm327_debug.argtypes = [POINTER(SimELM327)]
