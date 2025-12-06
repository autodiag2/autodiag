from autodiag.sim.elm327.elm327 import SimELM327
from autodiag.com.serial import Serial
from autodiag.com.vehicle_interface import VehicleIFace
from autodiag.log import *
from autodiag.sim.ecu.generator import *
from autodiag.lib import *

log_set_level(LOG_DEBUG)
emulation = SimELM327()
cycleGen = GeneratorCycle()
emulation.sim.ecus.contents.list[0].contents.set_generator(cycleGen)

emulation.loop(daemon=True)

serial = Serial()
serial.set_location(emulation.device_location)
iface = VehicleIFace.open_from_device(serial)