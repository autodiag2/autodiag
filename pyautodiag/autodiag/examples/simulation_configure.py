from autodiag.sim.elm327.elm327 import SimELM327
from autodiag.com.serial import Serial
from autodiag.com.obd.obd import OBDIFace
from autodiag.log import *
from autodiag.sim.elm327.sim_generators import *

log_set_level(LOG_DEBUG)
emulation = SimELM327()

@SimECUGenerator.CALLBACK_OBD_SIM_RESPONSE
def custom_sim_ecu_generator_response(generator_ptr, response_ptr, responseOBDdataBin, obd_query_bin):
    print(obd_query_bin)
    response_ptr[0] = c_char_p(b"OK")

class CustomECUGenerator(SimECUGenerator):
    def __init__(self):
        self.context = None
        self.sim_ecu_generator_response = custom_sim_ecu_generator_response
        self.type = "custom".encode()

from autodiag.lib import *
callback = SimECUGenerator.CALLBACK_OBD_SIM_RESPONSE(custom_sim_ecu_generator_response)
from ctypes import cast, c_void_p
print("Function address:", hex(cast(callback, c_void_p).value))
print(addr(byref(emulation.ecus.contents.list[0])))
emulation.debug()
emulation.debug_from_python()
emulation.ecus.contents.list[0].contents.set_generator(CustomECUGenerator())
emulation.debug()
emulation.debug_from_python()

emulation.loop(daemon=True)

import time
time.sleep(2)

print(f"Simulation started at: {emulation.device_location}")

serial = Serial()
serial.set_location(emulation.device_location)
iface = OBDIFace.open_from_device(serial)
iface.clear_data()
iface.send("0101")
iface.recv()