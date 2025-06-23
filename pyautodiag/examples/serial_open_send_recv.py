import sys
from autodiag.com.serial import Serial
from autodiag.log import log_set_level, LOG_DEBUG, LOG_NONE
from autodiag.sim.elm327.elm327 import SimELM327

emulation = SimELM327()
emulation.loop(daemon=True)

serial = Serial()
serial.set_location(emulation.device_location)
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
