import sys
from autodiag.com.serial import Serial
from autodiag.com.serial.elm.elm import ELMDevice
from autodiag.log import log_set_level, LOG_DEBUG, LOG_NONE

if len(sys.argv) < 2:
    print("Error: Please provide the port location as an argument.")
    sys.exit(1)

port_location = sys.argv[1]

log_set_level(LOG_DEBUG)
serial = Serial()
serial.name = port_location.encode('utf-8')
serial.debug()
serial.debug_from_python()
iface = ELMDevice.open_from_serial(serial)
iface.send("0101")
iface.recv()
iface.send("ati")
iface.recv()
iface.close()
