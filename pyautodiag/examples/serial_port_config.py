import sys
from autodiag.com.serial import Serial
from autodiag.log import log_set_level, LOG_DEBUG, LOG_NONE

if len(sys.argv) < 2:
    print("Error: Please provide the port location as an argument.")
    sys.exit(1)

log_set_level(LOG_DEBUG)
serial = Serial()
serial.set_location(sys.argv[1])
serial.open()
serial.send("ATE0")
serial.recv()
serial.recv_buffer.contents.recycle()
serial.send("ATL0")
serial.recv()
serial.recv_buffer.contents.recycle()
serial.eol = b'\r'
for i in range(10):
    serial.send("ATI")
    serial.recv()
    serial.recv_buffer.contents.recycle()
serial.close()
log_set_level(LOG_NONE)
