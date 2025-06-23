import sys
from autodiag.com.serial import Serial
from autodiag.log import log_set_level, LOG_DEBUG, LOG_NONE

from autodiag.sim.elm327.elm327 import SimELM327

emulation = SimELM327()
emulation.loop(daemon=True)

log_set_level(LOG_DEBUG)
serial = Serial()
serial.set_location(emulation.device_location)
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
