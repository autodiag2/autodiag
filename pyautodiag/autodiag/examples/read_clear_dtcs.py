import sys
from autodiag.com.serial import Serial
from autodiag.com.obd.obd import OBDIFace
from autodiag.com.obd.saej1979 import SAEJ1979
from autodiag.log import log_set_level, LOG_DEBUG, LOG_NONE

if len(sys.argv) < 2:
    print("Error: Please provide the port location as an argument.")
    sys.exit(1)

port_location = sys.argv[1]

log_set_level(LOG_DEBUG)
serial = Serial()
serial.name = port_location.encode('utf-8')
iface = OBDIFace.open_from_device(serial)

log_set_level(LOG_NONE)
saej1979 = SAEJ1979(iface)
print(f"number of dtcs before: {saej1979.number_of_dtc()}")
iface.send("04")
iface.recv()
print(f"number of dtcs after: {saej1979.number_of_dtc()}")
iface.close()
