import sys
from autodiag.com.serial import Serial
from autodiag.log import log_set_level, LOG_DEBUG, LOG_NONE
from autodiag.com.obd.obd import OBDIFace

if len(sys.argv) < 2:
    print("Error: Please provide the port location as an argument.")
    sys.exit(1)

log_set_level(LOG_DEBUG)
serial = Serial()
serial.set_port_location(sys.argv[1])
iface = OBDIFace.open_from_device(serial)
iface.send("0101")
iface.recv()
iface.vehicle.contents.dump()
iface.send("ati")
iface.recv()
iface.send("0102")
iface.recv()
iface.vehicle.contents.dump()
iface.close()
