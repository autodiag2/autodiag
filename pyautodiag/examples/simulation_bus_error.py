from autodiag.sim.elm327.elm327 import SimELM327
from autodiag.com.serial import Serial
from autodiag.com.obd.obd import OBDIFace
from autodiag.log import *
from autodiag.sim.elm327.sim_generators import *
from autodiag.lib import *

log_set_level(LOG_DEBUG)
emulation = SimELM327()
cycleGen = SimECUGeneratorCycle()
emulation.ecus.contents.list[0].contents.set_generator(cycleGen)

emulation.loop(daemon=True)

import time
time.sleep(2)

serial = Serial()
serial.set_location(emulation.device_location)
iface = OBDIFace.open_from_device(serial)