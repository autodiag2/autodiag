from autodiag.sim.elm327.elm327 import SimELM327
from autodiag.com.serial import Serial
from autodiag.com.obd.obd import OBDIFace
from autodiag.log import *

log_set_level(LOG_DEBUG)
emulation = SimELM327()
emulation.loop(daemon=True)

print(f"Simulation started at: {emulation.device_location}")

serial = Serial()
serial.set_location(emulation.device_location)
iface = OBDIFace.open_from_device(serial)
iface.clear_data()
iface.send("0101")
iface.recv()