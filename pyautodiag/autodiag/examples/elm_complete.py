import sys
from autodiag.com.serial import Serial
from autodiag.com.obd.obd import OBDIFace
from autodiag.log import log_set_level, LOG_DEBUG, LOG_NONE

from ctypes import cast, POINTER

if len(sys.argv) < 2:
    print("Error: Please provide the port location as an argument.")
    sys.exit(1)

port_location = sys.argv[1]

log_set_level(LOG_DEBUG)
serial = Serial()
serial.name = port_location.encode('utf-8')
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
