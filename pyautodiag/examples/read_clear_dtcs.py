import sys
from autodiag.com.serial import Serial
from autodiag.com.obd.obd import OBDIFace
from autodiag.com.obd.saej1979 import SAEJ1979
from autodiag.log import log_set_level, LOG_DEBUG, LOG_NONE

from autodiag.sim.elm327.elm327 import SimELM327

emulation = SimELM327()
emulation.loop(daemon=True)

log_set_level(LOG_DEBUG)
serial = Serial()
serial.set_location(emulation.device_location)
iface = OBDIFace.open_from_device(serial)

log_set_level(LOG_NONE)
saej1979 = SAEJ1979(iface)
print(f"number of dtcs before: {saej1979.number_of_dtc()}")
print(f"MIL: {saej1979.mil_status()}")
saej1979.clear_dtc_and_stored_values()
print(f"number of dtcs after: {saej1979.number_of_dtc()}")
print(f"MIL: {saej1979.mil_status()}")
iface.close()
