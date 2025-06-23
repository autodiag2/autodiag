import sys
from autodiag.com.serial import Serial
from autodiag.com.obd.obd import OBDIFace
from autodiag.log import log_set_level, LOG_DEBUG, LOG_NONE
from autodiag.sim.elm327.elm327 import SimELM327

emulation = SimELM327()
emulation.loop(daemon=True)

log_set_level(LOG_DEBUG)
serial = Serial()
serial.set_location(emulation.device_location)
iface = OBDIFace.open_from_device(serial)
iface.clear_data()
iface.send("0101")
iface.recv()
iface.vehicle.contents.dump()
iface.clear_data()
iface.send("ati")
iface.recv()
iface.vehicle.contents.dump()
iface.close()
