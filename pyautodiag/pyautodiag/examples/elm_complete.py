import sys
from pyautodiag.libautodiag.com.serial import Serial
from pyautodiag.libautodiag.com.serial.elm.elm import ELMDevice
from pyautodiag.libautodiag.log import log_set_level, LOG_DEBUG, LOG_NONE

if len(sys.argv) < 2:
    print("Error: Please provide the port location as an argument.")
    sys.exit(1)

port_location = sys.argv[1]

log_set_level(LOG_DEBUG)
serial = Serial()
serial.name = port_location.encode('utf-8')
iface = ELMDevice.open_from_serial(serial)
serial.close()
