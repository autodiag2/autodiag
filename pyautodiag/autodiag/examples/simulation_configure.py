from autodiag.sim.elm327.elm327 import ELM327emulation
from autodiag.com.serial import Serial
from autodiag.com.obd.obd import OBDIFace
from autodiag.log import *
from autodiag.sim.elm327.sim_generators import *

log_set_level(LOG_DEBUG)
emulation = ELM327emulation()
generator = ECUEmulationGenerator()
cycleGen = ECUEmulationGeneratorCycle()
assert emulation.ecus.contents.size == 1
emulation.ecus.contents.remove_at(0)
assert emulation.ecus.contents.size == 0
emulation.ecus.contents.append(cycleGen)
emulation.ecus.contents.append(generator)
assert emulation.ecus.contents.size == 2
emulation.ecus.contents.remove_at(0)
emulation.ecus.contents.remove_at(0)
assert emulation.ecus.contents.size == 0

@ECUEmulationGenerator.CALLBACK_OBD_SIM_RESPONSE
def custom_obd_sim_response(generator_ptr, response_ptr, responseOBDdataBin, obd_query_bin):
    print(obd_query_bin)
    response_ptr[0] = c_char_p(b"OK")

class CustomECUGenerator(ECUEmulationGenerator):
    def __init__(self):
        self.context = None
        self.obd_sim_response = custom_obd_sim_response
        self.type = "custom".encode()
emulation.ecus.contents.append(CustomECUGenerator())

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