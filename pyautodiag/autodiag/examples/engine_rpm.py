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
for i in range(10):
    print(saej1979.rpm())

iface.close()
