import sys
from autodiag.com.serial import Serial
from autodiag.com.vehicle_interface import VehicleIFace
from autodiag.com.obd.saej1979 import SAEJ1979
from autodiag.log import log_set_level, LOG_DEBUG, LOG_NONE
from autodiag.sim.elm327.elm327 import SimELM327

emulation = SimELM327()
emulation.loop(daemon=True)

serial = Serial()
serial.set_location(emulation.device_location)
iface = VehicleIFace.open_from_device(serial)

saej1979 = SAEJ1979(iface)
for i in range(10):
    print(saej1979.rpm())

iface.close()
