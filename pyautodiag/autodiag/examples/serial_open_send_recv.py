import sys
from autodiag.com.serial import Serial
from autodiag.log import log_set_level, LOG_DEBUG, LOG_NONE

if len(sys.argv) < 2:
    print("Error: Please provide the port location as an argument.")
    sys.exit(1)

serial = Serial()
serial.set_port_location(sys.argv[1])
log_set_level(LOG_DEBUG)
serial.dump()
log_set_level(LOG_NONE)
serial.open()
serial.send("ATI")
serial.recv()
log_set_level(LOG_DEBUG)
print(serial.describe_communication_layer())
print(serial.describe_status())
serial.recv_buffer.contents.dump()
log_set_level(LOG_NONE)
serial.close()
