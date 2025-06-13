import sys
from autodiag.com.serial import Serial
from autodiag.com.obd.obd import OBDIFace
from autodiag.com.obd.saej1979 import SAEJ1979
from autodiag.log import log_set_level, LOG_DEBUG, LOG_NONE

if len(sys.argv) < 2:
    print("Error: Please provide the port location as an argument.")
    sys.exit(1)

log_set_level(LOG_DEBUG)
serial = Serial()
serial.set_location(sys.argv[1])
iface = OBDIFace.open_from_device(serial)

log_set_level(LOG_NONE)
saej1979 = SAEJ1979(iface)
print(f"number of dtcs before: {saej1979.number_of_dtc()}")
print(f"MIL: {saej1979.mil_status()}")
saej1979.clear_dtc_and_stored_values()
print(f"number of dtcs after: {saej1979.number_of_dtc()}")
print(f"MIL: {saej1979.mil_status()}")
iface.close()
