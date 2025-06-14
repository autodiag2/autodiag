from autodiag.sim.elm327.elm327 import ELM327emulation
from autodiag.com.serial import Serial
from autodiag.com.obd.obd import OBDIFace
from autodiag.log import *
from autodiag.sim.elm327.sim_generators import ECUEmulationGenerator

log_set_level(LOG_DEBUG)
emulation = ELM327emulation()
generator = ECUEmulationGenerator()
print(generator.type)

import sys
sys.exit(1)
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